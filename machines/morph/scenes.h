#pragma once
#include <opxy/track_control.hpp>
#include "controls.h"

namespace morph {
// One parameter a scene sets: an instrument track, its live parameter index
// and the value.
struct Lock { opxy::u8 track, index; opxy::u16 reserved; opxy::i32 value; };
// Scenes 1..16 (slot 0 unused: scene 0 is "off"). Two full scenes fill the
// SDK's 64-entry overlay.
const unsigned kSceneLocks = 32;
struct Scene { opxy::u32 count; Lock locks[kSceneLocks]; };

inline const Lock* find(const Scene& scene, opxy::u32 track, opxy::u32 index) {
    for (opxy::u32 i = 0; i < scene.count; ++i)
        if (scene.locks[i].track == track && scene.locks[i].index == index) return &scene.locks[i];
    return nullptr;
}

// Records a value; a full scene keeps the parameters it already has.
inline void record(Scene& scene, opxy::u32 track, opxy::u32 index, opxy::i32 value) {
    Lock* lock = nullptr;
    for (opxy::u32 i = 0; i < scene.count && !lock; ++i)
        if (scene.locks[i].track == track && scene.locks[i].index == index) lock = &scene.locks[i];
    if (!lock) {
        if (scene.count >= kSceneLocks) return;
        lock = &scene.locks[scene.count++];
        lock->track = static_cast<opxy::u8>(track);
        lock->index = static_cast<opxy::u8>(index);
        lock->reserved = 0;
    }
    lock->value = value;
}
}  // namespace morph
