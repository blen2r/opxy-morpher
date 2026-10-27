#pragma once
#include <opxy/draw.hpp>

namespace morph {
// A crossfader of 21 ticks, one every 5% (nodes 0..20), and its thumb (21), the
// state A and B markers at its ends (22, 23), the write dot (24), and two red
// boxes around header values: the morph's (25) and an off state's (26). Two
// red text lines (0, 1) ask to remove extra copies. No paths.
const unsigned kTicks = 21, kThumb = 21, kMarkerA = 22, kMarkerB = 23, kWriteDot = 24;
const unsigned kMorphBox = 25, kStateBox = 26;
// Palette: black, dim, mid grey, white, red, then a black fill with a white or
// dim outline. The device changes a fill only on fill-and-stroke paint, so a
// marker always paints both and an unfilled marker paints black.
const opxy::u32 kBlack = 0, kDim = 1, kMid = 2, kWhite = 3, kRed = 4, kRingWhite = 5, kRingDim = 6;
using Frame = opxy::draw::Frame<0, 0, 27, 2, 7>;
const unsigned kCopiesTitle = 0, kCopiesHint = 1;
struct DisplayState { opxy::u32 unused; };
}
