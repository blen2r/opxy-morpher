// The morpher's page: where the morph sits between states A and B, which
// states are chosen, and whether an edit would be written to one of them.
#include <opxy/track_control.hpp>
#include "display.h"
#include "controls.h"

namespace morph {
// Geometry, in pixels: the shape in the middle at 1.5x, the state markers
// beside it, the crossfader's ticks from x = 80 to 400 along the bottom, the
// write dot at the top right.
const opxy::i32 kCentreX = 240, kCentreY = 112, kMarkerLeft = 40, kMarkerRight = 440;
const opxy::i32 kLeft = 80, kRight = 400, kRow = 204;
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

// The morph's percentage as the header shows it, with a % suffix.
inline void percent_text(char* out, unsigned capacity, unsigned p) {
    char text[5];
    number(text, sizeof(text) - 1, p);
    unsigned n = 0;
    while (text[n]) ++n;
    text[n++] = '%';
    text[n] = 0;
    opxy::draw::copy(out, capacity, text);
}
// The mode and state names, as the controls' labels give them. Stock shows an
// enum's raw 0..99 value; the header replaces it with the name.
const char* const kModeNames[kModes] = {"play", "wr a", "wr b"};
inline void state_name(char* out, unsigned capacity, unsigned state) {
    if (state)
        number(out, capacity, state);
    else
        opxy::draw::copy(out, capacity, "off");
}

inline void display_reset(DisplayState& state) { state.unused = 0; }

inline void display(DisplayState&, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    const unsigned mode = choice(input.slots[kModeSlot], kModes);
    const unsigned p = percent(input.slots[kMorphSlot]);
    const unsigned a = choice(input.slots[kStateASlot], kStates);
    const unsigned b = choice(input.slots[kStateBSlot], kStates);
    // Page 2 (a press switches pages): the frame option, then spare slots.
    const bool second = input.shown[0] >= kFrameSlot;
    if (second) {
        opxy::draw::copy(frame.header[0], sizeof(frame.header[0]), choice(input.slots[kFrameSlot], 2) ? "on" : "off");
        for (unsigned i = 1; i < 4; ++i) opxy::draw::copy(frame.header[i], sizeof(frame.header[i]), "-");
    } else {
        opxy::draw::copy(frame.header[kModeSlot], sizeof(frame.header[kModeSlot]), kModeNames[mode]);
        percent_text(frame.header[kMorphSlot], sizeof(frame.header[kMorphSlot]), p);
        state_name(frame.header[kStateASlot], sizeof(frame.header[kStateASlot]), a);
        state_name(frame.header[kStateBSlot], sizeof(frame.header[kStateBSlot]), b);
    }

    // Only one Morph runs: while another track holds one too, none does, and
    // every copy's page says so instead of showing the shape and crossfader.
    if (opxy::control::copies(input.track) > 1) {
        for (unsigned i = 0; i <= kStateBox; ++i) hide(frame, i);
        text(frame, kCopiesTitle, 72 * unit, 112 * unit, unit, "ONLY ONE MORPH ALLOWED");
        text(frame, kCopiesHint, 128 * unit, 148 * unit, unit, "remove the extra Morphs");
        return;
    }
    text(frame, kCopiesTitle, hidden_x * unit, hidden_y * unit, unit, "");
    text(frame, kCopiesHint, hidden_x * unit, hidden_y * unit, unit, "");

    // Writing plays the written state alone, so the morph is not in effect:
    // its thumb is hidden and its value boxed in red. Writing a side that is
    // off records nothing; that side's state is boxed in red instead.
    const unsigned states[2] = {a, b}, writes[2] = {kWriteA, kWriteB};
    const unsigned side = mode == kWriteB ? 1 : 0;
    const bool writing = mode != kPlay && states[side];

    // The shape follows the morph; while writing it shows the written state, in red.
    const opxy::i32 q15 = input.slots[kMorphSlot] < 0 ? 0 : input.slots[kMorphSlot] >> 16;
    shape(frame, writing ? (side ? 32768 : 0) : q15 >= 32767 ? 32768 : q15);
    place(frame, kShape, kCentreX * unit, kCentreY * unit, unit + unit / 2, writing ? kShapeRed : kShapeWhite, stroke);

    // The thumb sits where the shown percentage does, so at every 5% it is on a tick.
    const opxy::i32 thumb = kLeft + static_cast<opxy::i32>(p) * (kRight - kLeft) / 100;
    for (unsigned i = 0; i < kTicks; ++i) {
        const opxy::i32 x = kLeft + static_cast<opxy::i32>(i) * (kRight - kLeft) / (kTicks - 1);
        place(frame, kTick0 + i, x * unit, kRow * unit, unit, x <= thumb ? kWhite : kDim, fill_and_stroke);
    }
    if (writing)
        hide(frame, kThumb);
    else
        place(frame, kThumb, thumb * unit, kRow * unit, unit, kWhite, stroke);

    // A marker is outlined red while its state is being written, white when a
    // state is chosen, and dim when that side is off.
    const opxy::i32 xs[2] = {kMarkerLeft, kMarkerRight};
    for (unsigned s = 0; s < 2; ++s)
        place(frame, kMarkerA + s, xs[s] * unit, kCentreY * unit, unit,
              writing && mode == writes[s] ? kRingRed : states[s] ? kRingWhite : kRingDim,
              s ? stroke : fill_and_stroke);
    // The red boxes mark page 1's header values.
    if (writing)
        place(frame, kWriteDot, 456 * unit, 44 * unit, unit, kRed, fill_and_stroke);
    else
        hide(frame, kWriteDot);
    if (writing && !second)
        place(frame, kMorphBox, header_x(kMorphSlot) * unit, kHeaderY * unit, unit, kRed, stroke);
    else
        hide(frame, kMorphBox);
    if (mode != kPlay && !states[side] && !second)
        place(frame, kStateBox, header_x(side ? kStateBSlot : kStateASlot) * unit, kHeaderY * unit, unit, kRed, stroke);
    else
        hide(frame, kStateBox);
}
}
