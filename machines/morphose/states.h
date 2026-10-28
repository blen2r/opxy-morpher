#pragma once
#include <opxy/track_control.hpp>
#include "controls.h"

namespace morphose {
// One parameter a state sets: an instrument track, its live parameter index
// and the value.
struct Lock { opxy::u8 track, index; opxy::u16 reserved; opxy::i32 value; };
// States 1..16 (slot 0 unused: state 0 is "off"). Two full states fill the
// SDK's 64-entry overlay.
const unsigned kStateLocks = 32;
struct State { opxy::u32 count; Lock locks[kStateLocks]; };

inline const Lock* find(const State& state, opxy::u32 track, opxy::u32 index) {
    for (opxy::u32 i = 0; i < state.count; ++i)
        if (state.locks[i].track == track && state.locks[i].index == index) return &state.locks[i];
    return nullptr;
}

// Records a value; a full state keeps the parameters it already has.
inline void record(State& state, opxy::u32 track, opxy::u32 index, opxy::i32 value) {
    Lock* lock = nullptr;
    for (opxy::u32 i = 0; i < state.count && !lock; ++i)
        if (state.locks[i].track == track && state.locks[i].index == index) lock = &state.locks[i];
    if (!lock) {
        if (state.count >= kStateLocks) return;
        lock = &state.locks[state.count++];
        lock->track = static_cast<opxy::u8>(track);
        lock->index = static_cast<opxy::u8>(index);
        lock->reserved = 0;
    }
    lock->value = value;
}
}  // namespace morphose
