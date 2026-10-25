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

inline void display_reset(DisplayState& state) { state.unused = 0; }

inline void display(DisplayState&, const opxy::draw::Input& input, Frame& frame) {
    using namespace opxy::draw;
    const unsigned mode = choice(input.slots[kModeSlot], kModes);
    const unsigned step = position(input.slots[kMorphSlot]);
    const unsigned a = choice(input.slots[kSceneASlot], kScenes);
    const unsigned b = choice(input.slots[kSceneBSlot], kScenes);
    number(frame.header[kMorphSlot], sizeof(frame.header[kMorphSlot]), step);

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
              writing ? kRed : scenes[side] ? kWhite : kDim, writing ? fill_and_stroke : stroke);
    }
    if (mode == kPlay)
        hide(frame, kWriteDot);
    else
        place(frame, kWriteDot, 456 * unit, 44 * unit, unit, kRed, fill_and_stroke);
}
}
