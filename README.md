# Morph

An Octatrack-style scene morpher for the OP-XY, built on the
[Open-XY SDK](https://github.com/etcetc000/open-xy).

The morpher is a silent synth: put it on an instrument track and its page
has four controls.

| Control | Values |
| --- | --- |
| Mode | play, write A, write B: each detent of the knob moves one mode, and a press of the knob selects the next |
| Morph | 0% (all scene A) to 100% (all scene B); 50% is the exact midpoint |
| Scene A | off, 1–16 |
| Scene B | off, 1–16 |

"Off" leaves every parameter as it is on that side of the crossfader. All four
controls can be sequenced and p-locked like any other track parameter.

The page shows the crossfader, the two scene markers (dim when off) and, in a
write mode, the marker being written filled red and a red dot at the top right.

## Status

This is the page only. Applying the morph to other tracks' parameters,
capturing edits into scenes in write mode, an indicator on every screen and
saving scenes with the project need new SDK firmware hooks, which do not exist
yet.

## Checks

From this repository, with the SDK checked out beside it:

```sh
python ../open-xy/sdk.py test --machine machines/morph --firmware
```
