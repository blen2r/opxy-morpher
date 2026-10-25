// The morph itself, on the GUI side once per tick. In write A or write B,
// edits of other tracks' parameters are recorded into that side's scene; they
// play while writing and are undone when the mode leaves write. In play,
// every parameter scene A or B sets is overlaid with a blend of the two; a
// side that is off, or a scene that leaves a parameter alone, contributes the
// parameter's own value. Scenes live in RAM only, for every project.
#include <opxy/q31.hpp>
#include <opxy/track_control.hpp>
#include "controls.h"
#include "scenes.h"

namespace morph {
struct ControlState { Scene scenes[kScenes]; };

const Scene kEmpty = {0, {}};

// Scenes outlast a move to another track.
inline void control_reset(ControlState&) {}

// Overlays one parameter, unless the scene on the other side already did.
inline void blend(const opxy::control::Input& input, opxy::control::Overlay& overlay, const Scene& from,
                  const Scene& to, const Lock& lock, bool from_side, opxy::i32 morph) {
    const Lock* other = find(from_side ? to : from, lock.track, lock.index);
    if (!from_side && other) return;
    const opxy::i32 own = input.values[lock.track][lock.index];
    const opxy::i32 a = from_side ? lock.value : other ? other->value : own;
    const opxy::i32 b = from_side ? (other ? other->value : own) : lock.value;
    opxy::control::add(overlay, lock.track, lock.index, a + opxy::q31::multiply(b - a, morph));
}

inline void control(ControlState& state, const opxy::control::Input& input, opxy::control::Overlay& overlay) {
    const opxy::i32* own = input.values[input.track];
    const unsigned mode = choice(own[kModeSlot], kModes);
    const unsigned a = choice(own[kSceneASlot], kScenes), b = choice(own[kSceneBSlot], kScenes);
    if (mode != kPlay) {
        const unsigned target = mode == kWriteA ? a : b;
        if (!target) return;
        for (opxy::u32 i = 0; i < input.edit_count; ++i) {
            const opxy::control::Entry& e = input.edits[i];
            record(state.scenes[target], e.track, e.index, e.value);
        }
        overlay.capture = 1;
        return;
    }
    const Scene& from = a ? state.scenes[a] : kEmpty;
    const Scene& to = b ? state.scenes[b] : kEmpty;
    const opxy::i32 morph = own[kMorphSlot] < 0 ? 0 : own[kMorphSlot];
    for (opxy::u32 i = 0; i < from.count; ++i) blend(input, overlay, from, to, from.locks[i], true, morph);
    for (opxy::u32 i = 0; i < to.count; ++i) blend(input, overlay, from, to, to.locks[i], false, morph);
}
}  // namespace morph
