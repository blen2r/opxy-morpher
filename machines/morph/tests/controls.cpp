// The control decoding and the page's frame.
#include "../controls.h"
#include "../display.cpp"
#include <cstdio>

static int failures = 0;
static void expect(bool ok, const char* what) {
    if (!ok) { std::printf("FAIL %s\n", what); ++failures; }
}
static bool same(const char* a, const char* b) {
    while (*a && *a == *b) ++a, ++b;
    return *a == *b;
}
// The Q31 word for enum choice `index` of `count`, as the stock menu stores it.
static opxy::i32 word(unsigned index, unsigned count) {
    const unsigned q15 = (index * 32767u + (count - 1) / 2) / (count - 1);
    return static_cast<opxy::i32>(q15 << 16);
}

int main() {
    using namespace morph;
    for (unsigned i = 0; i < kModes; ++i) expect(choice(word(i, kModes), kModes) == i, "mode round trip");
    for (unsigned i = 0; i < kScenes; ++i) expect(choice(word(i, kScenes), kScenes) == i, "scene round trip");
    expect(choice(0x7fffffff, kScenes) == 16 && choice(-1, kScenes) == 0, "scene ends");
    expect(position(0) == 0 && position(0x40000000) == 64 && position(0x7fffffff) == 127, "morph scale");

    static Frame frame;
    static DisplayState state;
    opxy::draw::Input input = {};
    display_reset(state);
    for (unsigned mode = 0; mode < kModes; ++mode)
        for (unsigned s = 0; s < 3; ++s) {
            const opxy::i32 morph = s == 0 ? 0 : s == 1 ? 0x40000000 : 0x7fffffff;
            input.slots[kModeSlot] = input.parameters[0] = word(mode, kModes);
            input.slots[kMorphSlot] = input.parameters[1] = morph;
            input.slots[kSceneASlot] = input.parameters[2] = word(s * 5, kScenes);
            input.slots[kSceneBSlot] = input.parameters[3] = word(16 - s, kScenes);
            display(state, input, frame);
            expect(opxy::draw::valid(frame), "valid frame");
            const bool writing = mode != kPlay;
            expect((frame.nodes[kWriteDot].x == opxy::draw::hidden_x * opxy::draw::unit) == !writing, "write dot");
            expect((frame.nodes[kMarkerA].shade == kRed) == (mode == kWriteA), "marker A");
            expect((frame.nodes[kMarkerB].shade == kRed) == (mode == kWriteB), "marker B");
            // Both paint the fill, so a marker leaving red is repainted black.
            expect(frame.nodes[kMarkerA].paint == opxy::draw::fill_and_stroke &&
                   frame.nodes[kMarkerB].paint == opxy::draw::fill_and_stroke, "marker fill");
            static const char* const modes[kModes] = {"play", "wr a", "wr b"};
            static const char* const scenes[3][2] = {{"off", "16"}, {"5", "15"}, {"10", "14"}};
            expect(same(frame.header[kModeSlot], modes[mode]), "mode header");
            expect(same(frame.header[kSceneASlot], scenes[s][0]), "scene A header");
            expect(same(frame.header[kSceneBSlot], scenes[s][1]), "scene B header");
        }
    expect(frame.header[1][0] == '1' && frame.header[1][1] == '2' && frame.header[1][2] == '7' && !frame.header[1][3],
           "morph header");
    return failures ? 1 : 0;
}
