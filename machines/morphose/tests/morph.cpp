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
    using namespace morphose;
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
        for (unsigned i = 0; i < opxy::control::display_words; ++i) overlay.display[i] = 0;
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
    expect(bytes == 1 + 16 + locks * 6 + 1 && control_save(saved, data, kSavedBytes - 1) == 0, "save size");
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
    data[0] = 3;
    control_load(loaded, data, bytes);
    expect(loaded.states[1].count == 0 && loaded.states[3].count == 0, "unknown format starts empty");
    // Format 1, from before bypass, still loads, not bypassed.
    data[0] = 1;
    control_load(loaded, data, bytes - 1);
    expect(loaded.states[1].count == saved.states[1].count && !loaded.bypassed, "format 1 loads");
    data[0] = kSavedFormat;
    control_load(loaded, data, bytes - 1);
    expect(loaded.states[1].count == 0, "truncated data starts empty");
    control_load(loaded, data, bytes);
    control_load(loaded, data, 0);
    expect(loaded.states[1].count == 0 && loaded.states[3].count == 0, "project without states starts empty");

    // The LFO: shapes, depth around the knob, rates in real time and in time with the music.
    Lfo lfo = {};
    expect(wave(lfo, kSine) == 0 && wave(lfo, kTriangle) == 0 && wave(lfo, kRampUp) == -32767 &&
           wave(lfo, kSquare) == 32767, "shapes at the start");
    lfo.phase = 0x40000000;
    expect(wave(lfo, kSine) == 32767 && wave(lfo, kTriangle) > 32700, "shapes at a quarter");
    lfo.phase = 0xc0000000u;
    expect(wave(lfo, kSine) == -32767 && wave(lfo, kSquare) == -32767 && wave(lfo, kRampDown) == -16385,
           "shapes at three quarters");
    expect(modulate(0x40000000, 8192, 32767) > 0x5ff00000 && modulate(0x40000000, 8192, 32767) < 0x60000000 &&
           modulate(0x70000000, 32767, 32767) == 0x7fffffff && modulate(0x10000000, 32767, -32767) == 0,
           "depth around the knob, clamped");
    // Random picks evenly within the reachable swing: at morph 30% with full depth, every
    // fifth of 0..100% is about as likely, and almost nothing sits on 0.
    {
        unsigned fifths[5] = {}, zeros = 0;
        for (opxy::i32 held = -32768; held <= 32766; ++held) {
            const opxy::i32 m = modulate_random(0x26666666, 32767, held);
            expect(m >= 0, "random within range");
            zeros += m == 0;
            ++fifths[static_cast<unsigned>(m) / 0x1999999a];
        }
        expect(zeros <= 1, "random does not pile up at 0");
        for (unsigned f = 0; f < 5; ++f) expect(fifths[f] > 12000 && fifths[f] < 14300, "random spread evenly");
        // Within reach, the swing is the depth either side of the knob, whichever its sign.
        expect(modulate_random(0x40000000, 8192, -32768) > 0x1ff00000 && modulate_random(0x40000000, 8192, -32768) < 0x20100000 &&
               modulate_random(0x40000000, -8192, 32766) > 0x5fe00000 && modulate_random(0x40000000, -8192, 32766) <= 0x60000000,
               "random swing around the knob");
    }
    const opxy::Transport stopped = {1200, 0, false};
    lfo = {};
    for (unsigned i = 0; i < 100; ++i) advance(lfo, 7, 5000, stopped);  // 1 Hz for 0.5 s
    expect(lfo.phase > 0x7f000000 && lfo.phase < 0x81000000, "free rate in real time");
    lfo = {};
    for (unsigned i = 0; i < 100; ++i) advance(lfo, 21, 10000, stopped);  // 1 bar at 120 BPM is 2 s; 1 s here
    expect(lfo.phase > 0x7f000000 && lfo.phase < 0x81000000, "synced rate on the tempo while stopped");
    const opxy::Transport playing = {1200, 384 * 3 + 96, true};
    advance(lfo, 21, 10000, playing);
    expect(lfo.phase == 0x40000000, "synced rate on the song position while playing");
    const opxy::i32 first = lfo.held;
    lfo.phase = 0xffff0000u;
    advance(lfo, 7, 100000, stopped);
    expect(lfo.held != first && lfo.held >= -32768 && lfo.held <= 32766, "random value each cycle");

    // In play the LFO moves the morph and the page sees it; writing, it doesn't.
    own[kDepthSlot] = 0x60000000;
    own[kShapeSlot] = word(kSquare, kShapes);
    own[kRateSlot] = word(7, kRates);
    saved.lfo.phase = 0;
    input.elapsed_us = 0;
    run(kPlay, 1, 2, 0x40000000);
    expect(overlay.display[kShownLfo] == 1 && overlay.display[kShownMorph] > 0x5ff00000, "LFO moves the morph");
    run(kWriteA, 1, 2, 0x40000000);
    expect(overlay.display[kShownLfo] == 0, "no LFO while writing");
    own[kDepthSlot] = 0x40000000;
    run(kPlay, 1, 2, 0x40000000);
    expect(overlay.display[kShownLfo] == 0, "depth 0 is off");

    // M4 bypasses: nothing is overlaid or captured until it is pressed again;
    // the bypass is kept with the project.
    input.page_key = 4;
    run(kPlay, 1, 2, 0x40000000);
    input.page_key = 0;
    expect(overlay.count == 0 && overlay.display[kShownBypassed] == 1, "bypassed");
    run(kWriteA, 1, 2, 0);
    expect(overlay.count == 0 && !overlay.capture, "nothing written while bypassed");
    const opxy::u32 kept_bytes = control_save(saved, data, kSavedBytes);
    control_load(loaded, data, kept_bytes);
    expect(loaded.bypassed == 1, "bypass saved");
    input.page_key = 2;
    run(kPlay, 1, 2, 0x40000000);
    expect(overlay.display[kShownBypassed] == 1, "M2 is not bypass");
    input.page_key = 4;
    run(kPlay, 1, 2, 0x40000000);
    input.page_key = 0;
    expect(overlay.count > 0 && overlay.display[kShownBypassed] == 0, "bypass off again");
    return failures ? 1 : 0;
}
