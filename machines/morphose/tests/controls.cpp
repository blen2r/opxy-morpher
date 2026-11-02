// The control decoding and the page's frame.
#include "../controls.h"
#include "../display.cpp"
#include <cstdio>

// The SDK's count of tracks holding this machine, as the test sets it.
static opxy::u32 copies_now = 1;
extern "C" opxy::u32 sdk_control_copies(opxy::u32) { return copies_now; }
// What the control passed to the page, as the test sets it.
static opxy::i32 from_control_now[4];
extern "C" opxy::i32 sdk_control_display(opxy::u32 index) { return index < 4 ? from_control_now[index] : 0; }

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
    using namespace morphose;
    for (unsigned i = 0; i < 2; ++i) expect(choice(word(i, 2), 2) == i, "preview round trip");
    for (unsigned i = 0; i < kStates; ++i) expect(choice(word(i, kStates), kStates) == i, "state round trip");
    expect(choice(0x7fffffff, kStates) == 16 && choice(-1, kStates) == 0, "state ends");
    expect(percent(0) == 0 && percent(0x40000000) == 50 && percent(0x7fffffff) == 100 && percent(-1) == 0, "morph scale");

    static Frame frame;
    static DisplayState state;
    opxy::draw::Input input = {};
    display_reset(state);
    // Writing (the control's word): none, state A's, or a state neither side plays;
    // with preview off and on.
    for (unsigned preview = 0; preview < 2; ++preview)
        for (unsigned w = 0; w < 3; ++w)
            for (unsigned s = 0; s < 3; ++s) {
                const opxy::i32 morph = s == 0 ? 0 : s == 1 ? 0x40000000 : 0x7fffffff;
                const unsigned a = s * 5, written = w == 0 ? 0 : w == 1 ? a : 7;
                input.slots[kPreviewSlot] = input.parameters[0] = word(preview, 2);
                input.slots[kMorphSlot] = input.parameters[1] = morph;
                input.slots[kStateASlot] = input.parameters[2] = word(a, kStates);
                input.slots[kStateBSlot] = input.parameters[3] = word(16 - s, kStates);
                from_control_now[kShownWriting] = static_cast<opxy::i32>(written);
                display(state, input, frame);
                expect(opxy::draw::valid(frame), "valid frame");
                const bool writing = written != 0;
                const auto hidden = [&](unsigned node) { return frame.nodes[node].x == opxy::draw::hidden_x * opxy::draw::unit; };
                expect(hidden(kWriteDot) == !writing, "write dot");
                expect(hidden(kMorphBox) == !(writing && preview), "morph boxed while previewing");
                expect(hidden(kStateBox), "no off-state box");
                expect((frame.nodes[kMarkerA].shade == kRingRed) == (writing && written == a), "marker A");
                expect(frame.nodes[kMarkerB].shade != kRingRed, "marker B");
                // The circle paints its fill black; the square is an outline.
                expect(frame.nodes[kMarkerA].paint == opxy::draw::fill_and_stroke &&
                       frame.nodes[kMarkerB].paint == opxy::draw::stroke, "marker paint");
                expect(frame.nodes[kShape].shade == (writing ? kShapeRed : kShapeWhite), "shape colour");
                static const char* const states[3][2] = {{"off", "16"}, {"5", "15"}, {"10", "14"}};
                expect(same(frame.header[kPreviewSlot], preview ? "on" : "off"), "preview header");
                expect(same(frame.header[kStateASlot], states[s][0]), "state A header");
                expect(same(frame.header[kStateBSlot], states[s][1]), "state B header");
            }
    from_control_now[kShownWriting] = 0;
    input.slots[kPreviewSlot] = 0;
    expect(same(frame.header[kMorphSlot], "100"), "morph header at B");
    static const struct { opxy::i32 value; const char* text; } shown[] = {
        {0, "0"}, {0x40000000, "50"}, {0x3fffffff, "50"}, {0x147ae148, "16"}, {0x7fffffff, "100"}, {-1, "0"}};
    char text[opxy::draw::header_capacity];
    for (const auto& s : shown) {
        percent_text(text, sizeof(text), percent(s.value));
        expect(same(text, s.text), "morph percent");
    }
    // The shape: a circle at 0%, a square with exact corners at 100%, inside the path bounds.
    const auto point = [&](unsigned i) { return frame.paths[0].points[i]; };
    input.slots[kMorphSlot] = 0;
    display(state, input, frame);
    expect(point(0).x == 48 * 1024 && point(0).y == 0 && point(8).x == point(8).y && point(8).x == 724 * 48,
           "circle at 0%");
    input.slots[kMorphSlot] = 0x7fffffff;
    display(state, input, frame);
    expect(point(8).x == 42 * 1024 && point(8).y == 42 * 1024 && point(24).x == -42 * 1024 && point(0).x == 42 * 1024 &&
           point(4).x == 42 * 1024, "square with exact corners at 100%");
    const opxy::u32 square = frame.paths[0].key;
    input.slots[kMorphSlot] = 0x40000000;
    display(state, input, frame);
    expect(frame.paths[0].key != square && point(8).x > 724 * 48 && point(8).x < 42 * 1024, "between at 50%");
    expect(opxy::draw::valid(frame), "shape frame valid");

    // Page 2: the frame option and the LFO, without page 1's red boxes.
    from_control_now[kShownWriting] = 2;
    input.slots[kPreviewSlot] = word(1, 2);
    input.slots[kStateASlot] = word(0, kStates);
    input.slots[kFrameSlot] = word(1, 2);
    for (unsigned i = 0; i < 4; ++i) input.shown[i] = static_cast<opxy::u8>(kFrameSlot + i);
    display(state, input, frame);
    input.slots[kRateSlot] = word(21, kRates);
    input.slots[kDepthSlot] = 0x60000000;  // +50
    input.slots[kShapeSlot] = word(kSquare, kShapes);
    display(state, input, frame);
    expect(same(frame.header[0], "on") && same(frame.header[1], "1bar") && same(frame.header[2], "+50") &&
           same(frame.header[3], "sqr"), "page 2 header");
    input.slots[kDepthSlot] = 0x40000000;
    display(state, input, frame);
    expect(same(frame.header[2], "0"), "depth 0");
    input.slots[kDepthSlot] = 0;
    display(state, input, frame);
    expect(same(frame.header[2], "-100"), "depth -100");
    const opxy::i32 off_screen = opxy::draw::hidden_x * opxy::draw::unit;
    expect(frame.nodes[kStateBox].x == off_screen && frame.nodes[kMorphBox].x == off_screen, "no header boxes on page 2");
    input.slots[kFrameSlot] = 0;
    display(state, input, frame);
    expect(same(frame.header[0], "off"), "frame off");
    from_control_now[kShownWriting] = 0;
    // The two-row header: with page 2 edited, its values are the bright row (text
    // slots 4..7) and page 1's the grey row under them (12..15). A press animates
    // for 0.15 s; a reset (or a track change) snaps to the page shown.
    input.slots[kPreviewSlot] = 0;
    display_reset(state);
    display(state, input, frame);
    expect(same(frame.texts[4].chars, "off") && same(frame.texts[12].chars, "off") && same(frame.texts[0].chars, "frame") &&
           same(frame.texts[8].chars, "preview"), "two rows on page 2");
    for (unsigned i = 0; i < 4; ++i) input.shown[i] = static_cast<opxy::u8>(i);
    display(state, input, frame);
    // The covers hide the artwork's "only one Morphose" message, side by side over it.
    const opxy::i32 away = opxy::draw::hidden_x * opxy::draw::unit;
    bool covered = true;
    for (unsigned i = 0; i < kCovers; ++i)
        covered = covered && frame.nodes[kCover0 + i].x == (86 + 40 * static_cast<opxy::i32>(i)) * opxy::draw::unit &&
                  frame.nodes[kCover0 + i].y == 124 * opxy::draw::unit;
    expect(covered && frame.nodes[kLabelCover].x == 229 * opxy::draw::unit &&
           frame.nodes[kLabelCover + 1].x == 251 * opxy::draw::unit, "message and muted label covered");
    // Muted, the shape and markers dim and the label shows.
    from_control_now[kShownMuted] = 1;
    display(state, input, frame);
    expect(frame.nodes[kShape].shade == kShapeDim && frame.nodes[kMarkerA].shade == kRingDim &&
           frame.nodes[kLabelCover].x == away && frame.nodes[kLabelCover + 1].x == away, "muted look and label");
    from_control_now[kShownMuted] = 0;
    display(state, input, frame);
    // A second copy anywhere: every node moves away, uncovering the message.
    copies_now = 2;
    display(state, input, frame);
    bool hidden = true;
    for (unsigned i = 0; i < kLabelCover; ++i) hidden = hidden && frame.nodes[i].x == away;
    expect(hidden && frame.nodes[kLabelCover].x != away, "message shown, shape hidden with copies; label stays covered");
    expect(opxy::draw::valid(frame), "copies frame valid");
    // Selecting the other copy resets the display (and the frame): the
    // message must still be drawn, so the frame must be valid from scratch.
    static Frame fresh;
    display_reset(state);
    display(state, input, fresh);
    expect(opxy::draw::valid(fresh) && fresh.nodes[kCover0].x == away, "copies frame valid after a reset");
    copies_now = 1;
    display(state, input, frame);
    expect(frame.nodes[kCover0].x != away && frame.nodes[kShape].x != away, "covered again with one copy");
    return failures ? 1 : 0;
}
