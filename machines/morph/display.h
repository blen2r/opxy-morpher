#pragma once
#include <opxy/draw.hpp>

namespace morph {
// A shape between a circle (state A) and a square (state B), whose 64 points
// the morph moves (node 0, path 0); under it a crossfader of 21 ticks, one
// every 5% (nodes 1..21), and its thumb (22); the state A and B markers, a
// circle and a square (23, 24); the write dot (25); and two red boxes around
// header values: the morph's (26) and an off state's (27). Two red text lines
// (0, 1) ask to remove extra copies.
const unsigned kShape = 0, kTick0 = 1, kTicks = 21, kThumb = 22, kMarkerA = 23, kMarkerB = 24, kWriteDot = 25;
const unsigned kMorphBox = 26, kStateBox = 27, kNodes = 28, kShapePoints = 64;
// Palette: black, dim, mid grey, white, red, then a black fill with a white,
// dim or red outline, then the shape's white or red line. The device changes a
// fill only on fill-and-stroke paint, so a marker always paints both and an
// unfilled marker paints black.
const opxy::u32 kBlack = 0, kDim = 1, kMid = 2, kWhite = 3, kRed = 4, kRingWhite = 5, kRingDim = 6, kRingRed = 7;
const opxy::u32 kShapeWhite = 8, kShapeRed = 9;
using Frame = opxy::draw::Frame<1, kShapePoints, kNodes, 2, 10>;
const unsigned kCopiesTitle = 0, kCopiesHint = 1;
struct DisplayState { opxy::u32 unused; };
}
