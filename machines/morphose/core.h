#pragma once
#include <opxy/audio.hpp>

namespace morphose {
// The morpher makes no sound; a voice holds nothing.
struct Voice { opxy::u32 unused; };
struct Shared {};
}
