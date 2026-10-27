#pragma once
#include <opxy/audio.hpp>

namespace morph {
// Parameter slots (definition.json).
// Page 1: mode, morph and the two states; page 2 (a press switches pages): the
// frame on morphed tracks' pages, then three spare slots.
const unsigned kModeSlot = 0, kMorphSlot = 1, kStateASlot = 2, kStateBSlot = 3, kFrameSlot = 4;
// Modes, in the order of the mode control's labels.
const unsigned kPlay = 0, kWriteA = 1, kWriteB = 2, kModes = 3;
// State 0 is "off": that side leaves every parameter as it is. Then 1..16.
const unsigned kStates = 17;

// An enum control's index, quantized like the stock Q31-to-Q15 path.
inline unsigned choice(opxy::i32 value, unsigned count) {
    const unsigned index = value < 0 ? 0 : ((static_cast<unsigned>(value) >> 16) * count) >> 15;
    return index < count ? index : count - 1;
}
// The morph as a rounded percentage of the way from A to B, 0..100: 50 is the
// exact midpoint, where each side contributes half.
inline unsigned percent(opxy::i32 value) {
    return value < 0 ? 0 : ((static_cast<unsigned>(value) >> 7) * 100u + (1u << 23)) >> 24;
}
}  // namespace morph
