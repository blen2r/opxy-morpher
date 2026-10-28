#include "core.h"

namespace morphose {
// The morpher's track is a control track: its notes and parameters steer the
// morph, and its own output is silence.
inline void initialize(Voice& voice, Shared&, opxy::u32) { voice.unused = 0; }
inline void start(Voice&, Shared&, opxy::u32) {}
inline void destroy(Voice&, Shared&, opxy::u32) {}

inline void render(Voice&, Shared&, opxy::u32, opxy::Output output,
                   opxy::Parameters, opxy::Pitch) {
    for (unsigned i = 0; i < opxy::block_frames; ++i) output[i] = 0;
}
}
