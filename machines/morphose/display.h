#pragma once
#include <opxy/draw.hpp>
#include <opxy/page_header.hpp>

namespace morphose {
// A shape between a circle (state A) and a square (state B), whose 64 points
// the morph moves (node 0, path 0); the state A and B markers beside it, a
// circle and a square (1, 2); the write dot (3); two red boxes around header
// values: the morph's (4) and an off state's (5); nine black circles (6..14)
// that cover the artwork's "only one Morphose" message until two copies are
// placed; and two (15, 16) that cover the "muted" label above M4 except while
// the track is muted; then the SDK's two-row header (opxy/page_header.hpp):
// its eight bar nodes (17..24) and all 16 text slots. (Until 30b3d6c a
// crossfader of 21 ticks and a ring sat under the shape.)
const unsigned kShape = 0, kMarkerA = 1, kMarkerB = 2, kWriteDot = 3;
const unsigned kMorphBox = 4, kStateBox = 5, kCover0 = 6, kCovers = 9, kLabelCover = 15, kLabelCovers = 2, kHeaderBar = 17;
const unsigned kNodes = kHeaderBar + opxy::draw::page_header_nodes, kShapePoints = 64;
// Palette: black, dim, mid grey, white, red, then a black fill with a white,
// dim or red outline, then the shape's white or red line. The device changes a
// fill only on fill-and-stroke paint, so a marker always paints both and an
// unfilled marker paints black. The shape's line: white, red, or dim when muted.
const opxy::u32 kBlack = 0, kDim = 1, kMid = 2, kWhite = 3, kRed = 4, kRingWhite = 5, kRingDim = 6, kRingRed = 7;
const opxy::u32 kShapeWhite = 8, kShapeRed = 9, kShapeDim = 10;
using Frame = opxy::draw::Frame<1, kShapePoints, kNodes, opxy::draw::page_header_texts, 11>;
struct DisplayState { opxy::draw::PageHeader header; };
}
