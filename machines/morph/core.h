#pragma once
#include <opxy/audio.hpp>

namespace morph {
// The morpher makes no sound; a voice holds nothing.
struct Voice { opxy::u32 unused; };
struct Shared {};
}
