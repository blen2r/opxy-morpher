// The morph between scenes A and B, as the SDK's track control runs it.
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
static opxy::i32 near(opxy::i32 value, opxy::i32 expected) {
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
        control(state, input, overlay);
    };

    run(kPlay, 1, 2, 0);
    expect(overlay.count == 4 && overlay.entries[0].track == 0 && overlay.entries[0].value == 0, "all A");
    run(kPlay, 1, 2, 0x7fffffff);
    expect(overlay.count == 4 && near(overlay.entries[3].value, 0x7fffffff) <= 2, "all B");
    run(kPlay, 1, 2, 0x40000000);
    expect(overlay.count == 4 && near(overlay.entries[1].value, 0x40000000) <= 2, "midpoint");
    // An off side contributes the parameter's own value.
    run(kPlay, 0, 2, 0);
    expect(overlay.count == 4 && overlay.entries[0].value == 0x30000000, "A off");
    run(kPlay, 2, 0, 0x7fffffff);
    expect(overlay.count == 4 && near(overlay.entries[2].value, 0x30000000) <= 2, "B off");
    run(kPlay, 0, 0, 0x40000000);
    expect(overlay.count == 0, "both off");
    // Scenes 3..16 are empty; the same scene on both sides blends to itself.
    run(kPlay, 5, 6, 0x40000000);
    expect(overlay.count == 0, "empty scenes");
    run(kPlay, 2, 2, 0x20000000);
    expect(overlay.count == 4 && overlay.entries[0].value == 0x7fffffff, "same scene");
    run(kWriteA, 1, 2, 0x40000000);
    expect(overlay.count == 0, "write mode leaves parameters alone");
    return failures ? 1 : 0;
}
