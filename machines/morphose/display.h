#pragma once
#include <opxy/draw.hpp>

namespace morphose {
// A shape between a circle (state A) and a square (state B), whose 64 points
// the morph moves (node 0, path 0); the state A and B markers beside it, a
// circle and a square (1, 2); the write dot (3); two red boxes around header
// values: the morph's (4) and an off state's (5); nine black circles (6..14)
// that cover the artwork's "only one Morphose" message until two copies are
// placed; and two (15, 16) that cover the "ed" of M4's "bypassed" label
// except while bypassed. No text slot is used: a two-page header can have all 16. (Until 30b3d6c a crossfader of 21 ticks
// and a ring sat under the shape.)
const unsigned kShape = 0, kMarkerA = 1, kMarkerB = 2, kWriteDot = 3;
const unsigned kMorphBox = 4, kStateBox = 5, kCover0 = 6, kCovers = 9, kEdCover = 15, kEdCovers = 2, kNodes = 17, kShapePoints = 64;
// Palette: black, dim, mid grey, white, red, then a black fill with a white,
// dim or red outline, then the shape's white or red line. The device changes a
// fill only on fill-and-stroke paint, so a marker always paints both and an
// unfilled marker paints black. The shape's line: white, red, or dim when bypassed.
const opxy::u32 kBlack = 0, kDim = 1, kMid = 2, kWhite = 3, kRed = 4, kRingWhite = 5, kRingDim = 6, kRingRed = 7;
const opxy::u32 kShapeWhite = 8, kShapeRed = 9, kShapeDim = 10;
using Frame = opxy::draw::Frame<1, kShapePoints, kNodes, 0, 11>;
struct DisplayState { opxy::u32 unused; };
}
