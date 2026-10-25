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

## Writing scenes

Choose a scene number for A (or B), set the mode to write A (or write B), and
turn any encoder on another instrument track: each edit is recorded into that
scene, and you hear it while writing. Leaving write mode puts every edited
parameter back where it was. A scene holds up to 32 parameters.

## Playing

In play mode the morpher blends every parameter scene A or B sets on the other
instrument tracks. A side that is off, or a scene that leaves a parameter
alone, contributes the parameter's own value, so edits and parameter locks
still show through. The blend is never saved: the project keeps its own
values.

## Status

The morpher needs the SDK's track control (SDK 1.2), which is not in a
released SDK yet. Scenes live in memory only: they are lost when the OP-XY
turns off and are shared by every project. Edits made while holding a step
(parameter locks) are not recorded, and there is no way to clear a scene yet
except turning the OP-XY off. An indicator on every screen and saving scenes
with the project come next.

## Checks

From this repository, with the SDK checked out beside it:

```sh
python ../open-xy/sdk.py test --machine machines/morph --firmware
```
