#pragma once
#include <opxy/draw.hpp>

namespace morph {
// A crossfader of 17 ticks (nodes 0..16) and its thumb (17), the scene A and B
// markers at its ends (18, 19) and the write dot (20). No paths or text slots.
const unsigned kTicks = 17, kThumb = 17, kMarkerA = 18, kMarkerB = 19, kWriteDot = 20;
// Palette: black, dim, mid grey, white, red, then a black fill with a white or
// dim outline. The device changes a fill only on fill-and-stroke paint, so a
// marker always paints both and an unfilled marker paints black.
const opxy::u32 kBlack = 0, kDim = 1, kMid = 2, kWhite = 3, kRed = 4, kRingWhite = 5, kRingDim = 6;
using Frame = opxy::draw::Frame<0, 0, 21, 0, 7>;
struct DisplayState { opxy::u32 unused; };
}
