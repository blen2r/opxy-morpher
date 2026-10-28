// The morph itself, on the GUI side once per tick. In write A or write B,
// edits of other tracks' parameters are recorded into that side's state, which
// plays alone, as at morph 0% or 100%, whatever the morph control says; the
// edits are undone when the mode leaves write. Writing a side that is off
// records nothing and plays as in play mode. In play, every parameter state A
// or B sets is overlaid with a blend of the two; a side that is off, or a state
// that leaves a parameter alone, contributes the parameter's own value. States
// are kept with each project (control_save, control_load). Morph's own LFO
// (lfo.h) moves the morph in play.
#include <opxy/q31.hpp>
#include <opxy/track_control.hpp>
#include "controls.h"
#include "lfo.h"
#include "states.h"

namespace morph {
struct ControlState { State states[kStates]; Lfo lfo; opxy::u32 bypassed; };

const State kEmpty = {0, {}};

// States outlast a move to another track.
inline void control_reset(ControlState&) {}

// What a project keeps (definition.json's savedBytes): a format byte, then for
// states 1..16 their count and each parameter as track, index and the value's
// four bytes, low first, then (format 2) whether Morph is bypassed. Format 1,
// without the last byte, still loads.
const opxy::u8 kSavedFormat = 2;
const opxy::u32 kSavedBytes = 1 + (kStates - 1) * (1 + kStateLocks * 6) + 1;

inline opxy::u32 control_save(const ControlState& saved, opxy::u8* data, opxy::u32 capacity) {
    if (capacity < kSavedBytes) return 0;
    opxy::u32 n = 0;
    data[n++] = kSavedFormat;
    for (unsigned s = 1; s < kStates; ++s) {
        const State& state = saved.states[s];
        data[n++] = static_cast<opxy::u8>(state.count);
        for (opxy::u32 i = 0; i < state.count; ++i) {
            const Lock& lock = state.locks[i];
            const opxy::u32 value = static_cast<opxy::u32>(lock.value);
            data[n++] = lock.track;
            data[n++] = lock.index;
            for (unsigned b = 0; b < 4; ++b) data[n++] = static_cast<opxy::u8>(value >> (8 * b));
        }
    }
    data[n++] = saved.bypassed ? 1 : 0;
    return n;
}

// A project's states, or none: a project saved without them starts empty, and
// so does one whose data this version cannot read.
inline void control_load(ControlState& saved, const opxy::u8* data, opxy::u32 bytes) {
    for (unsigned s = 0; s < kStates; ++s) saved.states[s].count = 0;
    saved.bypassed = 0;
    if (!bytes || (data[0] != 1 && data[0] != kSavedFormat)) return;
    // The states end where the bypass byte, if any, begins.
    const opxy::u32 end = data[0] == kSavedFormat ? bytes - 1 : bytes;
    opxy::u32 n = 1;
    for (unsigned s = 1; s < kStates; ++s) {
        const opxy::u32 count = n < end ? data[n++] : kStateLocks + 1;
        if (count > kStateLocks || n + count * 6 > end || (s == kStates - 1 && n + count * 6 != end)) {
            for (unsigned t = 0; t < kStates; ++t) saved.states[t].count = 0;
            return;
        }
        for (opxy::u32 i = 0; i < count; ++i, n += 6) {
            const opxy::u32 value = data[n + 2] | data[n + 3] << 8 | data[n + 4] << 16 |
                                    static_cast<opxy::u32>(data[n + 5]) << 24;
            if (data[n] < opxy::control::tracks && data[n + 1] < opxy::control::track_parameters)
                record(saved.states[s], data[n], data[n + 1], static_cast<opxy::i32>(value));
        }
    }
    if (data[0] == kSavedFormat) saved.bypassed = data[end] ? 1 : 0;
}

// Overlays one parameter, unless the state on the other side already did.
inline void blend(const opxy::control::Input& input, opxy::control::Overlay& overlay, const State& from,
                  const State& to, const Lock& lock, bool from_side, opxy::i32 morph) {
    const Lock* other = find(from_side ? to : from, lock.track, lock.index);
    if (!from_side && other) return;
    const opxy::i32 own = input.values[lock.track][lock.index];
    const opxy::i32 a = from_side ? lock.value : other ? other->value : own;
    const opxy::i32 b = from_side ? (other ? other->value : own) : lock.value;
    opxy::control::add(overlay, lock.track, lock.index, a + opxy::q31::multiply(b - a, morph));
}

inline void control(ControlState& saved, const opxy::control::Input& input, opxy::control::Overlay& overlay) {
    const opxy::i32* own = input.values[input.track];
    const unsigned mode = choice(own[kModeSlot], kModes);
    const unsigned a = choice(own[kStateASlot], kStates), b = choice(own[kStateBSlot], kStates);
    const unsigned target = mode == kWriteA ? a : mode == kWriteB ? b : 0;
    // The LFO keeps time whatever the mode, and moves the morph only in play.
    advance(saved.lfo, choice(own[kRateSlot], kRates), input.elapsed_us, input.transport);
    // M4 on Morph's track bypasses it: nothing is morphed, written or marked,
    // and every track plays its own values.
    if (input.page_key == 4) saved.bypassed = !saved.bypassed;
    overlay.display[kShownBypassed] = static_cast<opxy::i32>(saved.bypassed);
    if (saved.bypassed) return;
    if (target) {
        State& state = saved.states[target];
        for (opxy::u32 i = 0; i < input.edit_count; ++i) {
            const opxy::control::Entry& e = input.edits[i];
            record(state, e.track, e.index, e.value);
        }
        for (opxy::u32 i = 0; i < state.count; ++i)
            opxy::control::add(overlay, state.locks[i].track, state.locks[i].index, state.locks[i].value);
        overlay.capture = 1;
        return;
    }
    // The tracks the morph changes show a thin frame, unless it is turned off.
    overlay.mark = choice(own[kFrameSlot], 2);
    const State& from = a ? saved.states[a] : kEmpty;
    const State& to = b ? saved.states[b] : kEmpty;
    opxy::i32 morph = own[kMorphSlot] < 0 ? 0 : own[kMorphSlot];
    const opxy::i32 d = depth(own[kDepthSlot]);
    if (d) {
        morph = modulate(morph, d, wave(saved.lfo, choice(own[kShapeSlot], kShapes)));
        overlay.display[kShownLfo] = 1;
        overlay.display[kShownMorph] = morph;
    }
    for (opxy::u32 i = 0; i < from.count; ++i) blend(input, overlay, from, to, from.locks[i], true, morph);
    for (opxy::u32 i = 0; i < to.count; ++i) blend(input, overlay, from, to, to.locks[i], false, morph);
}
}  // namespace morph
