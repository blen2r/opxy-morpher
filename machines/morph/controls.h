#pragma once
#include <opxy/audio.hpp>

namespace morph {
// Parameter slots (definition.json).
const unsigned kModeSlot = 0, kMorphSlot = 1, kSceneASlot = 2, kSceneBSlot = 3;
// Modes, in the order of the mode control's labels.
const unsigned kPlay = 0, kWriteA = 1, kWriteB = 2, kModes = 3;
// Scene 0 is "off": that side leaves every parameter as it is. Then 1..16.
const unsigned kScenes = 17;

// An enum control's index, quantized like the stock Q31-to-Q15 path.
inline unsigned choice(opxy::i32 value, unsigned count) {
    const unsigned index = value < 0 ? 0 : ((static_cast<unsigned>(value) >> 16) * count) >> 15;
    return index < count ? index : count - 1;
}
// The morph position on the Octatrack's 0..127 scale: 0 is all A, 64 the
// midpoint, 127 all B.
inline unsigned position(opxy::i32 value) { return value < 0 ? 0 : static_cast<unsigned>(value) >> 24; }
}  // namespace morph
