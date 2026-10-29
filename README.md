# Morphose

An Octatrack-style parameter morpher for the OP-XY, built on the
[Open-XY SDK](https://github.com/etcetc000/open-xy).

New to Morphose? Start with the [guide](GUIDE.md).

Morphose takes over auxiliary track 12 (CV), so it uses none of the eight sound
tracks: on track 12, hold Shift and press M1, then choose `morphose`. While it
holds the track, CV out and gate stay silent; choose `cv` the same way to bring
them back, with CV's knobs as they were. It has two pages
of four controls; pressing any knob switches between them. The header shows
both, page 1 above page 2, the one the knobs edit large and bright.

| Control | Values |
| --- | --- |
| Mode | play, write A, write B: each detent of the knob moves one mode |
| Morph | 0 (all state A) to 100 (all state B); 50 is the exact midpoint |
| State A | off, 1–16 |
| State B | off, 1–16 |
| Frame (page 2) | on, off: whether the pages of tracks Morphose is changing show a thin dark red frame |
| Rate (page 2) | the LFO's rate: free, 0.05 to 20 Hz, then synced to the tempo, 16 bars to 1/16 |
| Depth (page 2) | -100 to +100: how far the LFO swings the morph either side of the Morph knob; 0 is off |
| Shape (page 2) | sine, triangle, ramp up, ramp down, square, random |

**Presets:** Shift + track key on track 12 lists presets in two columns, as on
the sound tracks. Morphose comes with `init` (its defaults); add your own as
`aux_presets/<folder>/<name>.json` on the OP-XY over USB (MTP). Choosing one puts
Morphose on track 12 if `cv` was there.

**Bypass:** M4 on Morphose's track switches Morphose off and on (Morphose's page
labels it "bypass", just above M4). Bypassed, Morphose morphs, writes and
marks nothing, every track plays its own values, and its page is dimmed. The
bypass is saved with the project.

Morphose's LFO moves the morph in play mode, on every page; while writing it
pauses. Synced rates follow the song position while the sequencer plays, so
the LFO repeats the same way each pass, and the tempo while stopped. The page's
shape follows the LFO; the header shows the knob.

A state is a set of parameter values on other tracks, what the Octatrack calls
a scene (the OP-XY already uses "scene" for its sequencer).

"Off" leaves every parameter as it is on that side of the crossfader. All the
controls can be sequenced and p-locked like any other track parameter.

The page shows a shape that morphs from a circle (state A, at 0%) to a square
(state B, at 100%), and the two state markers beside it, a circle and a square
(dim when off). While writing, the shape shows the state being written, in
red, the marker being written is red, a red dot shows at the top right and the
morph's percentage is boxed in red.

Other pages show what Morphose is doing too: while you write a state, every page
has a red frame at the screen's edge, and the page of a track Morphose is
changing has a thin dark red frame (unless Frame is off).

## Writing states

Choose a state number for A (or B), set the mode to write A (or write B), and
turn any encoder on an instrument track: each edit is recorded into that
state. While writing you hear that state alone, as at morph 0% (or 100%), with
your edits, whatever the morph control says: its percentage is boxed in red
until you return to play, where it carries on from
where it was. Leaving write mode puts every edited parameter back where it
was. A state holds up to 32 parameters. Only the instrument tracks (1–8) are
recorded: knobs turned on the auxiliary tracks (9–16) are not.

Writing a side whose state is off records nothing: that state's "off" is boxed
in red, and the morph plays as in play mode.

## Playing

In play mode the morpher blends every parameter state A or B sets on the
instrument tracks (1–8). A side that is off, or a state that leaves a parameter
alone, contributes the parameter's own value, so edits and parameter locks
still show through. The blend is never saved: the project keeps its own
values.

### What wins: states, knobs and parameter locks

For each parameter on another track:

- **Neither state sets it:** the morpher leaves it alone. Knobs, parameter
  locks and LFOs work as usual.
- **A state sets it:** that side of the crossfader uses the state's value.
  The other side uses the other state's value, or, if that state doesn't set
  it (or is off), the track's own value at that moment, which includes any
  parameter lock playing on the current step.

So a parameter lock is heard only as far as the crossfader leans toward a
side that leaves the parameter alone. For example, state A sets noise to 99
and state B doesn't set noise; track 1 has noise locked to 0 on one step:

| Morph | Noise on the locked step | Noise on other steps |
| --- | --- | --- |
| 0% (all A) | 99 | 99 |
| 50% | halfway between 99 and 0 | halfway between 99 and the track's noise |
| 100% (all B) | 0 | the track's noise |

The same goes for turning that track's knob while morphing: the turn changes
the track's own value, which is saved with the project and comes back when
the morpher stops overriding it, but at 0% here you won't hear it. The
track's page shows the morphed value, and the track's own value while you
turn the knob.

Parameter locks are never written into a state, even in a write mode: they
stay locks on their step.

## Status

Morphose needs the SDK's track control (SDK 1.5) and aux tracks
([etcetc000/open-xy#220](https://github.com/etcetc000/open-xy/issues/220)),
which are not in a released SDK yet. States are saved with the project, in the companion file
beside it (`song.oxy` beside `song.xy`; copy both when you share a project),
and come back when it loads. A project saved without them, or on stock
firmware since, starts with every state empty. There is no way to clear a
single state yet.

## Why track 12

Morphose changes other tracks and makes no sound, so it belongs on an auxiliary
track rather than one of the eight sound tracks. The OP-XY's auxiliary tracks
each have a fixed role (brain, punch-in FX, MIDI, CV, audio in, tape), so a
machine there replaces that role, and the SDK has to keep the role silent.
CV is the first track the SDK does that for: its output is a few calls the SDK
skips while a machine holds the track. Version 1.0.0 of Morphose ran on a sound
track instead.

## Why Morphose doesn't use the OP-XY's LFO

For reviewers. On the OP-XY, an LFO on a synth's control is evaluated by the
audio side for voices that are sounding, and, for a track with none, only
while that track is selected. Morphose makes no sound, so stock's LFO moved the
morph only while Morphose's page was shown. We traced this in the firmware
(OS 1.2.0 and 1.2.1), forced the selected-track check, and held notes on
Morphose's track: stock never evaluated the LFO for Morphose's track while another
track was shown. So Morphose hides the stock LFO page and runs its own LFO
on the morph, on its second page, computed on the GUI side every tick, where
it works whichever page is shown.

## Checks

From this repository, with the SDK checked out beside it:

```sh
python ../open-xy/sdk.py test --machine machines/morphose --firmware
```

On an OP-XY, [QA.md](QA.md) lists the checks by area; running every section is
the end-to-end test.
