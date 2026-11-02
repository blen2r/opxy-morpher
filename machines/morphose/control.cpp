// The morph itself, on the GUI side once per tick. Metronome + a black key
// starts writing that state (the SDK's metronome_chord claim): edits of other
// tracks' parameters are recorded into it, and every page shows the state's
// values. Silently by default: what plays doesn't change, the morph included,
// and the edits are undone when writing stops; with preview on, the state
// plays alone while it is written. While writing, Morphose takes the
// metronome: tapping it alone stops writing, and another black key switches
// to that state. In play, every parameter state A or B sets is overlaid with
// a blend of the two; a side that is off, or a state that leaves a parameter
// alone, contributes the parameter's own value; while a state is written
// silently, the morph plays it as it was before. On Morphose's track, holding a
// black key plays that state alone (the keyboard, trackControl.keyboard).
// States are kept with each project (control_save, control_load). Morphose's
// own LFO (lfo.h) moves the morph in play.
#include <opxy/chord_tap.hpp>
#include <opxy/draw.hpp>
#include <opxy/keys.hpp>
#include <opxy/q31.hpp>
#include <opxy/track_control.hpp>
#include "controls.h"
#include "lfo.h"
#include "states.h"

namespace morphose {
struct ControlState {
    State states[kStates];
    Lfo lfo;
    unsigned writing;     // the state being written, or 0
    State before;         // that state when writing it started, which the morph plays meanwhile
    opxy::ChordTap tap;   // the metronome's gestures
    unsigned punch;       // the state a held key on Morphose's track plays, or 0
    opxy::u32 punch_key;  // that key
};

const State kEmpty = {0, {}};

// The keyboard's black keys, lowest first, choose states 1 to 10: the keyboard
// runs up from F at key 26 (stock's key numbers, opxy/keys.hpp).
const opxy::u8 kBlackKeys[] = {27, 29, 31, 34, 36, 39, 41, 43, 46, 48};
inline unsigned state_of_key(opxy::u32 key) {
    for (unsigned i = 0; i < sizeof(kBlackKeys); ++i)
        if (kBlackKeys[i] == key) return i + 1;
    return 0;
}

// Text with a number at its end, as a popup or a mark shows it.
inline void say(char* out, unsigned capacity, const char* words, unsigned n) {
    unsigned at = 0;
    while (words[at] && at + 4 < capacity) { out[at] = words[at]; ++at; }
    if (n >= 10) out[at++] = static_cast<char>('0' + n / 10);
    out[at++] = static_cast<char>('0' + n % 10);
    out[at] = 0;
}

// States outlast a move to another track; writing and a held key don't.
inline void control_reset(ControlState& saved) {
    saved.writing = saved.punch = 0;
    saved.tap = opxy::ChordTap();
}

// What a project keeps (definition.json's savedBytes): a format byte (1), then
// for states 1..16 their count and each parameter as track, index and the
// value's four bytes, low first. Format 2 (v1.0-v1.1) added whether Morphose was
// bypassed, now its track's mute: it still loads, without that byte.
const opxy::u8 kSavedFormat = 1, kBypassFormat = 2;
const opxy::u32 kSavedBytes = 1 + (kStates - 1) * (1 + kStateLocks * 6);

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
    return n;
}

// A project's states, or none: a project saved without them starts empty, and
// so does one whose data this version cannot read.
inline void control_load(ControlState& saved, const opxy::u8* data, opxy::u32 bytes) {
    for (unsigned s = 0; s < kStates; ++s) saved.states[s].count = 0;
    saved.writing = saved.punch = 0;
    if (!bytes || (data[0] != kSavedFormat && data[0] != kBypassFormat)) return;
    // The states end where format 2's bypass byte begins.
    const opxy::u32 end = data[0] == kBypassFormat ? bytes - 1 : bytes;
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
            if (data[n] < opxy::control::all_tracks && data[n + 1] < opxy::control::track_parameters)
                record(saved.states[s], data[n], data[n + 1], static_cast<opxy::i32>(value));
        }
    }
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

// What the morph plays for a state: as it was before, while it is written.
inline const State& playing(const ControlState& saved, unsigned state) {
    return state == saved.writing ? saved.before : saved.states[state];
}

// The metronome's gestures and the keyboard's on Morphose's track.
inline void gestures(ControlState& saved, const opxy::control::Input& input, opxy::control::Overlay& overlay) {
    for (opxy::u32 i = 0; i < input.gesture_count; ++i) {
        const opxy::control::Gesture& g = input.gestures[i];
        const unsigned state = state_of_key(g.key);
        if (g.lead == opxy::keys::metronome) {
            const opxy::ChordTap::Result r = saved.tap.feed(g.lead, g.key, g.pressed != 0);
            if (r == opxy::ChordTap::chord && state) {
                saved.writing = state;
                saved.before = saved.states[state];
                say(overlay.popup, opxy::control::popup_bytes, "writing to state ", state);
            } else if (r == opxy::ChordTap::chord) {
                opxy::draw::copy(overlay.popup, opxy::control::popup_bytes, "black keys pick states");
            } else if (r == opxy::ChordTap::tap && saved.writing) {
                say(overlay.popup, opxy::control::popup_bytes, "done writing state ", saved.writing);
                saved.writing = 0;
            }
        } else if (g.lead == 0 && g.pressed && state) {
            saved.punch = state;
            saved.punch_key = g.key;
        } else if (g.lead == 0 && !g.pressed && g.key == saved.punch_key) {
            saved.punch = 0;
        }
    }
}

inline void control(ControlState& saved, const opxy::control::Input& input, opxy::control::Overlay& overlay) {
    const opxy::i32* own = input.values[input.track];
    const unsigned a = choice(own[kStateASlot], kStates), b = choice(own[kStateBSlot], kStates);
    // The LFO keeps time whatever happens, and moves the morph only in play.
    advance(saved.lfo, choice(own[kRateSlot], kRates), input.elapsed_us, input.transport);
    // Muting Morphose's track switches it off, as it does stock's brain: nothing
    // is morphed, written or marked (the SDK applies none of it), and every track
    // plays its own values. Its page says so.
    overlay.display[kShownMuted] = static_cast<opxy::i32>(input.muted);
    gestures(saved, input, overlay);
    overlay.display[kShownWriting] = static_cast<opxy::i32>(saved.writing);
    if (input.muted) return;
    if (saved.writing) {
        State& state = saved.states[saved.writing];
        for (opxy::u32 i = 0; i < input.edit_count; ++i) {
            const opxy::control::Entry& e = input.edits[i];
            record(state, e.track, e.index, e.value);
        }
        const bool preview = choice(own[kPreviewSlot], 2) != 0;
        overlay.capture = preview ? 1 : 2;
        overlay.take = 1;
        say(overlay.mark_text, opxy::control::mark_text_bytes, "", saved.writing);
        for (opxy::u32 i = 0; i < state.count; ++i) {
            const Lock& lock = state.locks[i];
            opxy::control::show(overlay, lock.track, lock.index, lock.value);
            if (preview) opxy::control::add(overlay, lock.track, lock.index, lock.value);
        }
        // Previewing plays the state alone; otherwise the morph plays on.
        if (preview) return;
    }
    // The tracks the morph changes show a thin frame, unless it is turned off.
    overlay.mark = choice(own[kFrameSlot], 2);
    // A held key on Morphose's track plays its state alone.
    const bool punched = saved.punch && !saved.writing;
    const State& from = punched ? saved.states[saved.punch] : a ? playing(saved, a) : kEmpty;
    const State& to = punched ? kEmpty : b ? playing(saved, b) : kEmpty;
    opxy::i32 morph = punched ? 0 : own[kMorphSlot] < 0 ? 0 : own[kMorphSlot];
    const opxy::i32 d = depth(own[kDepthSlot]);
    if (d && !punched) {
        const unsigned shape = choice(own[kShapeSlot], kShapes);
        morph = shape == kRandom ? modulate_random(morph, d, saved.lfo.held) : modulate(morph, d, wave(saved.lfo, shape));
        overlay.display[kShownLfo] = 1;
        overlay.display[kShownMorph] = morph;
    }
    for (opxy::u32 i = 0; i < from.count; ++i) blend(input, overlay, from, to, from.locks[i], true, morph);
    for (opxy::u32 i = 0; i < to.count; ++i) blend(input, overlay, from, to, to.locks[i], false, morph);
}
}  // namespace morphose
