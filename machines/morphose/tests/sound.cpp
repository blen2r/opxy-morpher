// The machine's own checks, compiled for the host and run by
// python sdk.py test --machine <folder>.
#include "../engine.cpp"
#include <cstdio>

int main() {
    static morphose::Voice voice;
    static morphose::Shared shared;
    const opxy::i32 settings[3][8] = {{0}, {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff},
                                      {0x40000000, 0x40000000, 0x10000000, 0x60000000}};
    opxy::u32 pitch[opxy::block_frames];
    for (unsigned i = 0; i < opxy::block_frames; ++i) pitch[i] = 42852281u;  // 440 Hz at 44.1 kHz
    opxy::i32 output[opxy::block_frames];
    morphose::initialize(voice, shared, 1);
    morphose::start(voice, shared, 1);
    bool silent = true;
    for (unsigned s = 0; s < 3; ++s)
        for (unsigned block = 0; block < 690; ++block) {  // one second each
            for (unsigned i = 0; i < opxy::block_frames; ++i) output[i] = 0x1234;
            morphose::render(voice, shared, 1, output, settings[s], pitch);
            for (unsigned i = 0; i < opxy::block_frames; ++i) silent = silent && output[i] == 0;
        }
    if (!silent) std::puts("FAIL the morpher made sound");
    return silent ? 0 : 1;
}
