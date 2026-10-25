// Writing scenes from captured edits, and the morph between scenes A and B,
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
    static ControlState state;
    control_reset(state);
    input.track = 3;
    for (unsigned i = 0; i < 4; ++i) input.values[0][i] = 0x30000000;
    opxy::i32* own = input.values[3];
    const auto run = [&](unsigned mode, unsigned a, unsigned b, opxy::i32 position) {
        own[kModeSlot] = word(mode, kModes);
        own[kSceneASlot] = word(a, kScenes);
        own[kSceneBSlot] = word(b, kScenes);
        own[kMorphSlot] = position;
        overlay.count = 0;
        overlay.capture = 0;
        control(state, input, overlay);
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
    expect(overlay.count == 0 && !overlay.capture, "empty scenes");
    // Writing a side with scene off captures nothing.
    run(kWriteA, 0, 2, 0);
    expect(!overlay.capture, "write to off");
    // Write A into scene 1: edits are captured and recorded, nothing overlaid.
    edit(0, 1, 0x10000000);
    edit(0, 2, 0x20000000);
    edit(0, 1, 0);
    run(kWriteA, 1, 2, 0);
    expect(overlay.capture && overlay.count == 0 && state.scenes[1].count == 2, "write A");
    edit(0, 2, 0x7fffffff);
    run(kWriteB, 1, 2, 0);
    expect(overlay.capture && state.scenes[2].count == 1, "write B");

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
    expect(overlay.count == 1 && overlay.entries[0].value == 0x7fffffff, "same scene");
    // Scenes outlast a reset; a full scene keeps what it has.
    control_reset(state);
    for (unsigned i = 0; i < 20; ++i) edit(1, i, 5);
    run(kWriteA, 3, 0, 0);
    for (unsigned i = 20; i < 40; ++i) edit(1, i, 5);
    run(kWriteA, 3, 0, 0);
    expect(state.scenes[1].count == 2 && state.scenes[3].count == kSceneLocks, "full scene");
    return failures ? 1 : 0;
}
