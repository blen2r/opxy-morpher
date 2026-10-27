// Writing states from captured edits, and the morph between states A and B,
// as the SDK's track control runs them.
#include "../control.cpp"
#include <cstdio>

static int failures = 0;
static void expect(bool ok, const char* what) {
    if (!ok) { std::printf("FAIL %s\n", what); ++failures; }
}
static opxy::i32 word(unsigned index, unsigned count) {
    const unsigned q15 = (index * 32767u + (count - 1) / 2) / (count - 1);
    return static_cast<opxy::i32>(q15 << 16);
}
static opxy::i32 distance(opxy::i32 value, opxy::i32 expected) {
    const opxy::i32 d = value - expected;
    return d < 0 ? -d : d;
}

int main() {
    using namespace morph;
    static opxy::control::Input input;
    static opxy::control::Overlay overlay;
    static ControlState saved;
    control_reset(saved);
    input.track = 3;
    for (unsigned i = 0; i < 4; ++i) input.values[0][i] = 0x30000000;
    opxy::i32* own = input.values[3];
    const auto run = [&](unsigned mode, unsigned a, unsigned b, opxy::i32 position) {
        own[kModeSlot] = word(mode, kModes);
        own[kStateASlot] = word(a, kStates);
        own[kStateBSlot] = word(b, kStates);
        own[kMorphSlot] = position;
        overlay.count = 0;
        overlay.capture = 0;
        overlay.mark = 0;
        control(saved, input, overlay);
        input.edit_count = 0;
    };
    const auto edit = [&](unsigned track, unsigned index, opxy::i32 value) {
        opxy::control::Entry& e = input.edits[input.edit_count++];
        e.track = static_cast<opxy::u8>(track);
        e.index = static_cast<opxy::u8>(index);
        e.reserved = 0;
        e.value = value;
    };

    run(kPlay, 1, 2, 0x40000000);
    expect(overlay.count == 0 && !overlay.capture, "empty states");
    // Writing a side with state off captures nothing.
    run(kWriteA, 0, 2, 0);
    expect(!overlay.capture, "write to off");
    // Write A into state 1: edits are captured and recorded, and state 1 plays
    // alone whatever the morph, its new edits included.
    edit(0, 1, 0x10000000);
    edit(0, 2, 0x20000000);
    edit(0, 1, 0);
    run(kWriteA, 1, 2, 0x7fffffff);
    expect(overlay.capture && saved.states[1].count == 2 && overlay.count == 2, "write A");
    expect(overlay.entries[0].index == 1 && overlay.entries[0].value == 0 &&
           overlay.entries[1].index == 2 && overlay.entries[1].value == 0x20000000, "write A plays state A");
    edit(0, 2, 0x7fffffff);
    run(kWriteB, 1, 2, 0);
    expect(overlay.capture && saved.states[2].count == 1, "write B");
    expect(overlay.count == 1 && overlay.entries[0].value == 0x7fffffff, "write B plays state B alone");
    // Writing an off side records nothing and plays as in play mode.
    edit(0, 3, 5);
    run(kWriteA, 0, 2, 0x7fffffff);
    expect(!overlay.capture && overlay.count == 1 && distance(overlay.entries[0].value, 0x7fffffff) <= 2, "write to off plays");
    expect(!find(saved.states[2], 0, 3), "write to off records nothing");

    run(kPlay, 1, 2, 0);
    expect(!overlay.capture && overlay.count == 2, "play overlays the union");
    expect(overlay.entries[0].index == 1 && overlay.entries[0].value == 0, "all A, A's value");
    expect(overlay.entries[1].index == 2 && overlay.entries[1].value == 0x20000000, "all A, shared parameter");
    run(kPlay, 1, 2, 0x7fffffff);
    expect(distance(overlay.entries[0].value, 0x30000000) <= 2, "all B, parameter only A sets");
    expect(distance(overlay.entries[1].value, 0x7fffffff) <= 2, "all B, B's value");
    run(kPlay, 1, 2, 0x40000000);
    expect(distance(overlay.entries[1].value, 0x50000000) <= 2, "midpoint");
    // An off side contributes the parameter's own value.
    run(kPlay, 0, 2, 0);
    expect(overlay.count == 1 && overlay.entries[0].value == 0x30000000, "A off");
    run(kPlay, 2, 2, 0x20000000);
    expect(overlay.count == 1 && overlay.entries[0].value == 0x7fffffff, "same state");
    // The frame control marks the morphed tracks' pages, on unless turned off.
    expect(!overlay.mark, "frame off by its zero word");
    own[kFrameSlot] = word(1, 2);
    run(kPlay, 2, 2, 0x20000000);
    expect(overlay.mark == 1, "frame on");
    // States outlast a reset; a full state keeps what it has.
    control_reset(saved);
    for (unsigned i = 0; i < 20; ++i) edit(1, i, 5);
    run(kWriteA, 3, 0, 0);
    for (unsigned i = 20; i < 40; ++i) edit(1, i, 5);
    run(kWriteA, 3, 0, 0);
    expect(saved.states[1].count == 2 && saved.states[3].count == kStateLocks, "full state");

    // States go with the project: saved, loaded back, and emptied by a project without them.
    static opxy::u8 data[opxy::control::saved_capacity];
    const opxy::u32 bytes = control_save(saved, data, kSavedBytes);
    opxy::u32 locks = 0;
    for (unsigned s = 1; s < kStates; ++s) locks += saved.states[s].count;
    expect(bytes == 1 + 16 + locks * 6 && control_save(saved, data, kSavedBytes - 1) == 0, "save size");
    static ControlState loaded;
    control_load(loaded, data, bytes);
    bool same = true;
    for (unsigned s = 0; s < kStates; ++s) {
        same = same && loaded.states[s].count == saved.states[s].count;
        for (unsigned i = 0; i < saved.states[s].count; ++i) {
            const Lock &x = loaded.states[s].locks[i], &y = saved.states[s].locks[i];
            same = same && x.track == y.track && x.index == y.index && x.value == y.value;
        }
    }
    expect(same, "states round trip");
    data[0] = 2;
    control_load(loaded, data, bytes);
    expect(loaded.states[1].count == 0 && loaded.states[3].count == 0, "unknown format starts empty");
    data[0] = kSavedFormat;
    control_load(loaded, data, bytes - 1);
    expect(loaded.states[1].count == 0, "truncated data starts empty");
    control_load(loaded, data, bytes);
    control_load(loaded, data, 0);
    expect(loaded.states[1].count == 0 && loaded.states[3].count == 0, "project without states starts empty");
    return failures ? 1 : 0;
}
