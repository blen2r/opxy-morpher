// The morph itself, on the GUI side once per tick. In write A or write B,
// edits of other tracks' parameters are recorded into that side's state, which
// plays alone, as at morph 0% or 100%, whatever the morph control says; the
// edits are undone when the mode leaves write. Writing a side that is off
// records nothing and plays as in play mode. In play, every parameter state A
// or B sets is overlaid with a blend of the two; a side that is off, or a state
// that leaves a parameter alone, contributes the parameter's own value. States
// live in RAM only, for every project.
#include <opxy/q31.hpp>
#include <opxy/track_control.hpp>
#include "controls.h"
#include "states.h"

namespace morph {
struct ControlState { State states[kStates]; };

const State kEmpty = {0, {}};

// States outlast a move to another track.
inline void control_reset(ControlState&) {}

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
    const State& from = a ? saved.states[a] : kEmpty;
    const State& to = b ? saved.states[b] : kEmpty;
    const opxy::i32 morph = own[kMorphSlot] < 0 ? 0 : own[kMorphSlot];
    for (opxy::u32 i = 0; i < from.count; ++i) blend(input, overlay, from, to, from.locks[i], true, morph);
    for (opxy::u32 i = 0; i < to.count; ++i) blend(input, overlay, from, to, to.locks[i], false, morph);
}
}  // namespace morph
