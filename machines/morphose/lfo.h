#pragma once
#include <opxy/transport.hpp>
#include <opxy/types.hpp>

namespace morphose {
// Morphose's own LFO on the morph position (page 2: rate, depth, shape). Stock's
// LFO cannot move Morphose off its page (see the README), so Morphose runs this
// one on the GUI side every tick.

// Rates, slow to fast: free rates in hundredths of a hertz, then rates synced
// to the tempo, as 96ths of a quarter note per cycle (16 bars to 1/16).
const unsigned kFreeRates = 17, kSyncedRates = 9, kRates = kFreeRates + kSyncedRates;
const opxy::u32 kCentiHertz[kFreeRates] = {5, 10, 15, 20, 30, 50, 75, 100, 150, 200, 300, 400, 500, 700, 1000, 1400, 2000};
const opxy::u32 kTicksPerCycle[kSyncedRates] = {6144, 3072, 1536, 768, 384, 192, 96, 48, 24};
// The header's names for them: free rates in hertz, synced ones in bars or notes.
const char* const kRateNames[kRates] = {"0.05", "0.1", "0.15", "0.2", "0.3", "0.5", "0.75", "1", "1.5", "2",
                                        "3", "4", "5", "7", "10", "14", "20", "16bar", "8bar", "4bar",
                                        "2bar", "1bar", "1/2", "1/4", "1/8", "1/16"};
enum Shape { kSine, kTriangle, kRampUp, kRampDown, kSquare, kRandom, kShapes };
const char* const kShapeNames[kShapes] = {"sine", "tri", "up", "down", "sqr", "rand"};

struct Lfo {
    opxy::u32 phase;   // the cycle, 2^32 per cycle
    opxy::i32 held;    // the random shape's value this cycle, Q15
    opxy::u32 seed;
};

// Moves the phase on: synced rates follow the song position while the
// sequencer plays, and the tempo otherwise; free rates follow real time.
// Increments are per microsecond, 2^32 per cycle, wrapping as the phase does.
inline void advance(Lfo& lfo, unsigned rate, opxy::u32 elapsed_us, const opxy::Transport& transport) {
    const opxy::u32 before = lfo.phase;
    if (rate < kFreeRates) {
        // 2^32 / 1e6 / 100 per centi-hertz is 42.95.
        lfo.phase += elapsed_us * (kCentiHertz[rate] * 4295 / 100);
    } else {
        const opxy::u32 ticks = kTicksPerCycle[rate - kFreeRates];
        if (transport.playing && transport.bpm_x10) {
            lfo.phase = (((transport.ticks96 % ticks) << 16) / ticks) << 16;
        } else if (transport.bpm_x10) {
            // 2^32 * 96 / 600 / 1e6 per tenth of a BPM is 687.2, divided by the cycle's ticks.
            lfo.phase += elapsed_us * (transport.bpm_x10 * 6872 / (ticks * 10));
        }
    }
    if (lfo.phase < before) {  // a new cycle: a new random value
        lfo.seed = lfo.seed * 1664525u + 1013904223u;
        lfo.held = (static_cast<opxy::i32>(lfo.seed >> 17) - 16384) * 2;
    }
}

// The shape at the phase, -32767..32767; every shape but the ramps and the
// square starts its cycle at 0, rising.
inline opxy::i32 wave(const Lfo& lfo, unsigned shape) {
    const opxy::i32 p = static_cast<opxy::i32>(lfo.phase >> 16);  // 0..65535
    opxy::i32 y;
    switch (shape) {
    case kSine: {
        // A parabola per half cycle, within 6% of a sine.
        const opxy::i32 x = p - 32768, m = x < 0 ? -x : x;
        y = -((4 * x * (32768 - m)) >> 15);
        break;
    }
    case kTriangle: {
        const opxy::i32 t = (p + 16384) & 0xffff;
        y = (t < 32768 ? t : 65535 - t) * 2 - 32768;
        break;
    }
    case kRampUp: y = p - 32768; break;
    case kRampDown: y = 32767 - p; break;
    case kSquare: y = p < 32768 ? 32767 : -32767; break;
    default: y = lfo.held; break;
    }
    return y < -32767 ? -32767 : y > 32767 ? 32767 : y;
}

// The morph moved by the LFO: depth is -32767..32767 of the whole range, the
// result clamped to 0..100%.
inline opxy::i32 modulate(opxy::i32 morph, opxy::i32 depth, opxy::i32 y) {
    const opxy::i32 delta = (depth * y) << 1;
    if (delta > 0 && morph > 0x7fffffff - delta) return 0x7fffffff;
    const opxy::i32 out = morph + delta;
    return out < 0 ? 0 : out;
}
}  // namespace morphose
