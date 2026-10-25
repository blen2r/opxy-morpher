// The morph itself: on the GUI side, once per tick, every parameter scene A or
// B sets is overlaid with a blend of the two. A side that is off, or a scene
// that leaves a parameter alone, contributes the parameter's own value.
#include <opxy/q31.hpp>
#include <opxy/track_control.hpp>
#include "controls.h"
#include "scenes.h"

namespace morph {
struct ControlState { opxy::u32 unused; };

inline void control_reset(ControlState& state) { state.unused = 0; }

inline const Lock* find(const Scene& scene, opxy::u32 track, opxy::u32 index) {
    for (opxy::u32 i = 0; i < scene.count; ++i)
        if (scene.locks[i].track == track && scene.locks[i].index == index) return &scene.locks[i];
    return nullptr;
}

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

inline void control(ControlState&, const opxy::control::Input& input, opxy::control::Overlay& overlay) {
    const opxy::i32* own = input.values[input.track];
    if (choice(own[kModeSlot], kModes) != kPlay) return;
    const Scene a = scene(choice(own[kSceneASlot], kScenes)), b = scene(choice(own[kSceneBSlot], kScenes));
    const opxy::i32 morph = own[kMorphSlot] < 0 ? 0 : own[kMorphSlot];
    for (opxy::u32 i = 0; i < a.count; ++i) blend(input, overlay, a, b, a.locks[i], true, morph);
    for (opxy::u32 i = 0; i < b.count; ++i) blend(input, overlay, a, b, b.locks[i], false, morph);
}
}  // namespace morph
