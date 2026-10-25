// The morpher's page: where the morph sits between scenes A and B, which
// scenes are chosen, and whether an edit would be written to one of them.
#include "display.h"
#include "controls.h"

namespace morph {
// Crossfader geometry, in pixels: ticks from x = 80 to 400 on y = 120, scene
// markers beyond each end, the write dot at the top right.
const opxy::i32 kLeft = 80, kRight = 400, kRow = 120, kMarkerLeft = 40, kMarkerRight = 440;

inline void number(char* out, unsigned capacity, unsigned value) {
    char text[4];
    unsigned n = 0;
    if (value >= 100) text[n++] = static_cast<char>('0' + value / 100);
    if (value >= 10) text[n++] = static_cast<char>('0' + value / 10 % 10);
    text[n++] = static_cast<char>('0' + value % 10);
    text[n] = 0;
    opxy::draw::copy(out, capacity, text);
}

// The morph as a rounded percentage of the way from A to B: 50% is the exact
// midpoint, where each side contributes half.
inline void percent(char* out, unsigned capacity, opxy::i32 value) {
    const unsigned p = value < 0 ? 0 : ((static_cast<unsigned>(value) >> 7) * 100u + (1u << 23)) >> 24;
    char text[5];
    number(text, sizeof(text) - 1, p);
    unsigned n = 0;
    while (text[n]) ++n;
    text[n++] = '%';
    text[n] = 0;
    opxy::draw::copy(out, capacity, text);
}
// The mode and scene names, as the controls' labels give them. Stock shows an
// enum's raw 0..99 value; the header replaces it with the name.
const char* const kModeNames[kModes] = {"play", "wr a", "wr b"};
inline void scene_name(char* out, unsigned capacity, unsigned scene) {
    if (scene)
        number(out, capacity, scene);
    else
        opxy::draw::copy(out, capacity, "off");
}

inline void display_reset(DisplayState& state) { state.unused = 0; }

inline void display(DisplayState&, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    const unsigned mode = choice(input.slots[kModeSlot], kModes);
    const unsigned step = position(input.slots[kMorphSlot]);
    const unsigned a = choice(input.slots[kSceneASlot], kScenes);
    const unsigned b = choice(input.slots[kSceneBSlot], kScenes);
    opxy::draw::copy(frame.header[kModeSlot], sizeof(frame.header[kModeSlot]), kModeNames[mode]);
    percent(frame.header[kMorphSlot], sizeof(frame.header[kMorphSlot]), input.slots[kMorphSlot]);
    scene_name(frame.header[kSceneASlot], sizeof(frame.header[kSceneASlot]), a);
    scene_name(frame.header[kSceneBSlot], sizeof(frame.header[kSceneBSlot]), b);

    const opxy::i32 thumb = kLeft + static_cast<opxy::i32>(step) * (kRight - kLeft) / 127;
    for (unsigned i = 0; i < kTicks; ++i) {
        const opxy::i32 x = kLeft + static_cast<opxy::i32>(i) * (kRight - kLeft) / (kTicks - 1);
        place(frame, i, x * unit, kRow * unit, unit, x <= thumb ? kWhite : kDim, fill_and_stroke);
    }
    place(frame, kThumb, thumb * unit, kRow * unit, unit, kWhite, stroke);

    // A marker is filled red while its scene is being written, outlined white
    // when a scene is chosen, and dim when that side is off.
    const unsigned scenes[2] = {a, b}, writes[2] = {kWriteA, kWriteB};
    const opxy::i32 xs[2] = {kMarkerLeft, kMarkerRight};
    for (unsigned side = 0; side < 2; ++side) {
        const bool writing = mode == writes[side];
        place(frame, kMarkerA + side, xs[side] * unit, kRow * unit, unit,
              writing ? kRed : scenes[side] ? kRingWhite : kRingDim, fill_and_stroke);
    }
    if (mode == kPlay)
        hide(frame, kWriteDot);
    else
        place(frame, kWriteDot, 456 * unit, 44 * unit, unit, kRed, fill_and_stroke);
}
}
