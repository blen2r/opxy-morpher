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
    expect(percent(0) == 0 && percent(0x40000000) == 50 && percent(0x7fffffff) == 100 && percent(-1) == 0, "morph scale");

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
            // Scene A is off when s is 0; scene B never is.
            const bool writing = mode == kWriteB || (mode == kWriteA && s);
            const auto hidden = [&](unsigned node) { return frame.nodes[node].x == opxy::draw::hidden_x * opxy::draw::unit; };
            expect(hidden(kWriteDot) == !writing, "write dot");
            expect(hidden(kThumb) == writing, "thumb hidden while writing");
            expect(hidden(kMorphBox) == !writing, "morph boxed while writing");
            expect(hidden(kSceneBox) == (mode == kPlay || writing), "off scene boxed while written");
            expect(hidden(kSceneBox) || frame.nodes[kSceneBox].x == 330 * opxy::draw::unit, "scene A's box");
            expect((frame.nodes[kMarkerA].shade == kRed) == (writing && mode == kWriteA), "marker A");
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
    expect(same(frame.header[kMorphSlot], "100%"), "morph header at B");
    static const struct { opxy::i32 value; const char* text; } shown[] = {
        {0, "0%"}, {0x40000000, "50%"}, {0x3fffffff, "50%"}, {0x147ae148, "16%"}, {0x7fffffff, "100%"}, {-1, "0%"}};
    char text[opxy::draw::header_capacity];
    for (const auto& s : shown) {
        percent_text(text, sizeof(text), percent(s.value));
        expect(same(text, s.text), "morph percent");
    }
    // At every 5% the thumb sits exactly on a tick, the ticks below it lit.
    input.slots[kModeSlot] = word(kPlay, kModes);
    for (unsigned i = 0; i < kTicks; ++i) {
        input.slots[kMorphSlot] = static_cast<opxy::i32>((static_cast<unsigned long long>(i) * 0x7fffffff + 10) / 20);
        display(state, input, frame);
        expect(frame.nodes[kThumb].x == frame.nodes[i].x, "thumb on its tick");
        for (unsigned j = 0; j < kTicks; ++j) expect((frame.nodes[j].shade == kWhite) == (j <= i), "lit ticks");
    }
    return failures ? 1 : 0;
}
