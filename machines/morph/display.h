#pragma once
#include <opxy/draw.hpp>

namespace morph {
// A shape between a circle (state A) and a square (state B), whose 64 points
// the morph moves (node 0, path 0); the state A and B markers beside it, a
// circle and a square (1, 2); the write dot (3); and two red boxes around
// header values: the morph's (4) and an off state's (5). Two red text lines
// (0, 1) ask to remove extra copies. (Until 30b3d6c a crossfader of 21 ticks
// and a ring sat under the shape.)
const unsigned kShape = 0, kMarkerA = 1, kMarkerB = 2, kWriteDot = 3;
const unsigned kMorphBox = 4, kStateBox = 5, kNodes = 6, kShapePoints = 64;
// Palette: black, dim, mid grey, white, red, then a black fill with a white,
// dim or red outline, then the shape's white or red line. The device changes a
// fill only on fill-and-stroke paint, so a marker always paints both and an
// unfilled marker paints black. The shape's line: white, red, or dim when bypassed.
const opxy::u32 kBlack = 0, kDim = 1, kMid = 2, kWhite = 3, kRed = 4, kRingWhite = 5, kRingDim = 6, kRingRed = 7;
const opxy::u32 kShapeWhite = 8, kShapeRed = 9, kShapeDim = 10;
using Frame = opxy::draw::Frame<1, kShapePoints, kNodes, 3, 11>;
// Text slots: the copies message's two lines, and M4's label at the bottom
// right, above the M4 key.
const unsigned kCopiesTitle = 0, kCopiesHint = 1, kBypassLabel = 2;
struct DisplayState { opxy::u32 unused; };
}
