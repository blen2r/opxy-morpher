#pragma once
#include <opxy/track_control.hpp>
#include "controls.h"

namespace morph {
// One parameter a scene sets: an instrument track, its live parameter index
// and the value.
struct Lock { opxy::u8 track, index; opxy::u16 reserved; opxy::i32 value; };
struct Scene { opxy::u32 count; const Lock* locks; };

// Fixed test scenes until edits can be captured: scene 1 turns track 1's four
// engine controls fully down, scene 2 fully up. Scenes 3..16 are empty.
const Lock kDown[] = {{0, 0, 0, 0}, {0, 1, 0, 0}, {0, 2, 0, 0}, {0, 3, 0, 0}};
const Lock kUp[] = {{0, 0, 0, 0x7fffffff}, {0, 1, 0, 0x7fffffff}, {0, 2, 0, 0x7fffffff}, {0, 3, 0, 0x7fffffff}};
inline Scene scene(unsigned number) {
    const Scene none = {0, nullptr}, down = {4, kDown}, up = {4, kUp};
    return number == 1 ? down : number == 2 ? up : none;
}
}  // namespace morph
