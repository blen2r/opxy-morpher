// The morpher's page: where the morph sits between states A and B, which
// states are chosen, and whether an edit would be written to one of them.
#include <opxy/track_control.hpp>
#include "display.h"
#include "controls.h"
#include "lfo.h"

namespace morphose {
// Geometry, in pixels: the shape in the middle at 1.5x, the state markers
// beside it, the write dot at the top right.
// Below the two-row header, which takes the top 38 pixels.
const opxy::i32 kCentreX = 240, kCentreY = 124, kMarkerLeft = 40, kMarkerRight = 440;
// The shape's circle radius and square half-side, chosen for about the same area.
const opxy::i32 kRadius = 48, kHalfSide = 42;
// cos(k * 2 pi / 64) for k = 0..16, Q10.
const opxy::i32 kCos[17] = {1024, 1019, 1004, 980, 946, 903, 851, 792, 724, 650, 569, 483, 392, 297, 200, 100, 0};
inline opxy::i32 cos64(unsigned k) {
    k %= 64;
    return k <= 16 ? kCos[k] : k <= 32 ? -kCos[32 - k] : k <= 48 ? -kCos[k - 32] : kCos[64 - k];
}

// The shape at `t` of the way from the circle to the square (Q15, 32768 is the
// square): each point moves straight from the circle to the square point at
// its angle, so the square's corners are exact.
inline void shape(Frame& frame, opxy::i32 t) {
    auto& path = frame.paths[0];
    if (path.key == static_cast<opxy::u32>(t) + 1) return;
    path.key = static_cast<opxy::u32>(t) + 1;
    for (unsigned i = 0; i < kShapePoints; ++i) {
        const opxy::i32 c = cos64(i), s = cos64(i + 48);
        const opxy::i32 m = (c < 0 ? -c : c) > (s < 0 ? -s : s) ? (c < 0 ? -c : c) : (s < 0 ? -s : s);
        const opxy::i32 cx = kRadius * c, cy = kRadius * s;
        const opxy::i32 sx = kHalfSide * c * 1024 / m, sy = kHalfSide * s * 1024 / m;
        // Points move under 12 pixels (Q10), so 12 bits of t keep the product in 32 bits.
        path.points[i].x = cx + ((sx - cx) * (t >> 3) >> 12);
        path.points[i].y = cy + ((sy - cy) * (t >> 3) >> 12);
    }
}
// The centre of control position i's value in the header: its 60-pixel cell
// from x = 60 + 120 i, 20 pixels tall.
inline opxy::i32 header_x(unsigned i) { return 90 + 120 * static_cast<opxy::i32>(i); }
const opxy::i32 kHeaderY = 10;

inline void number(char* out, unsigned capacity, unsigned value) {
    char text[4];
    unsigned n = 0;
    if (value >= 100) text[n++] = static_cast<char>('0' + value / 100);
    if (value >= 10) text[n++] = static_cast<char>('0' + value / 10 % 10);
    text[n++] = static_cast<char>('0' + value % 10);
    text[n] = 0;
    opxy::draw::copy(out, capacity, text);
}

// The morph as the header shows it: 0 to 100, without a percent sign, as the
// OP-XY shows amounts.
inline void percent_text(char* out, unsigned capacity, unsigned p) { number(out, capacity, p); }
// The mode and state names, as the controls' labels give them. Stock shows an
// enum's raw 0..99 value; the header replaces it with the name.
const char* const kModeNames[kModes] = {"play", "wr a", "wr b"};
inline void state_name(char* out, unsigned capacity, unsigned state) {
    if (state)
        number(out, capacity, state);
    else
        opxy::draw::copy(out, capacity, "off");
}

inline void display_reset(DisplayState& state) { state.header.reset(); }

inline void display(DisplayState& state, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    const unsigned mode = choice(input.slots[kModeSlot], kModes);
    const unsigned p = percent(input.slots[kMorphSlot]);
    const unsigned a = choice(input.slots[kStateASlot], kStates);
    const unsigned b = choice(input.slots[kStateBSlot], kStates);
    // The SDK's two-row header: page 1 (mode, morph, the states) on top, page 2
    // (frame and the LFO) under it; the row the encoders edit is the large one.
    const bool second = input.shown[0] >= kFrameSlot;
    const char* const titles[8] = {"mode", "morph", "state a", "state b", "frame", "rate", "depth", "shape"};
    char values[8][header_capacity] = {};
    opxy::draw::copy(values[kModeSlot], header_capacity, kModeNames[mode]);
    percent_text(values[kMorphSlot], header_capacity, p);
    state_name(values[kStateASlot], header_capacity, a);
    state_name(values[kStateBSlot], header_capacity, b);
    opxy::draw::copy(values[kFrameSlot], header_capacity, choice(input.slots[kFrameSlot], 2) ? "on" : "off");
    opxy::draw::copy(values[kRateSlot], header_capacity, kRateNames[choice(input.slots[kRateSlot], kRates)]);
    // Depth as -100..100.
    const opxy::i32 d = depth(input.slots[kDepthSlot]);
    const unsigned amount = static_cast<unsigned>((d < 0 ? -d : d) * 100 + 16383) / 32767;
    values[kDepthSlot][0] = amount ? (d < 0 ? '-' : '+') : 0;
    number(values[kDepthSlot] + (amount ? 1 : 0), header_capacity - 1, amount);
    opxy::draw::copy(values[kShapeSlot], header_capacity, kShapeNames[choice(input.slots[kShapeSlot], kShapes)]);
    page_header(state.header, input, frame, titles, values, kHeaderBar);

    // Only one Morphose runs: while another track holds one too, none does, and
    // every copy's page says so instead of showing the shape.
    // The "muted" label in the middle of the shape shows only while the track
    // is muted: two circles drawn over it cover it otherwise.
    const bool off = opxy::control::from_control(kShownMuted) != 0;
    for (unsigned i = 0; i < kLabelCovers; ++i)
        if (off)
            hide(frame, kLabelCover + i);
        else
            place(frame, kLabelCover + i, (231 + 18 * static_cast<opxy::i32>(i)) * unit, kCentreY * unit, unit, kBlack,
                  fill_and_stroke);
    if (opxy::control::copies(input.track) > 1) {
        // A frame is drawn only once its path has points (a nonzero key),
        // and the SDK clears them whenever the track or page changes.
        shape(frame, 0);
        for (unsigned i = 0; i < kLabelCover; ++i) hide(frame, i);
        return;
    }
    // The covers sit over the message, the rest is drawn over them.
    for (unsigned i = 0; i < kCovers; ++i)
        place(frame, kCover0 + i, (86 + 40 * static_cast<opxy::i32>(i)) * unit, kCentreY * unit, unit, kBlack, fill_and_stroke);

    // Writing plays the written state alone, so the morph is not in effect:
    // its value is boxed in red. Writing a side that is
    // off records nothing; that side's state is boxed in red instead.
    const unsigned states[2] = {a, b}, writes[2] = {kWriteA, kWriteB};
    const unsigned side = mode == kWriteB ? 1 : 0;
    const bool writing = !off && mode != kPlay && states[side];

    // The shape follows the morph; while writing it shows the written state, in red.
    // The shape shows the morph as the LFO moves it, the header the knob.
    const opxy::i32 moved = opxy::control::from_control(kShownLfo) ? opxy::control::from_control(kShownMorph)
                                                                   : input.slots[kMorphSlot];
    const opxy::i32 q15 = moved < 0 ? 0 : moved >> 16;
    shape(frame, writing ? (side ? 32768 : 0) : q15 >= 32767 ? 32768 : q15);
    place(frame, kShape, kCentreX * unit, kCentreY * unit, unit + unit / 2,
          off ? kShapeDim : writing ? kShapeRed : kShapeWhite, stroke);

    // A marker is outlined red while its state is being written, white when a
    // state is chosen, and dim when that side is off.
    const opxy::i32 xs[2] = {kMarkerLeft, kMarkerRight};
    for (unsigned s = 0; s < 2; ++s)
        place(frame, kMarkerA + s, xs[s] * unit, kCentreY * unit, unit,
              writing && mode == writes[s] ? kRingRed : states[s] && !off ? kRingWhite : kRingDim,
              s ? stroke : fill_and_stroke);
    // The red boxes mark page 1's header values.
    if (writing)
        place(frame, kWriteDot, 456 * unit, 56 * unit, unit, kRed, fill_and_stroke);
    else
        hide(frame, kWriteDot);
    if (writing && !second)
        place(frame, kMorphBox, header_x(kMorphSlot) * unit, kHeaderY * unit, unit, kRed, stroke);
    else
        hide(frame, kMorphBox);
    if (!off && mode != kPlay && !states[side] && !second)
        place(frame, kStateBox, header_x(side ? kStateBSlot : kStateASlot) * unit, kHeaderY * unit, unit, kRed, stroke);
    else
        hide(frame, kStateBox);
}
}
