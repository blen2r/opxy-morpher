# Morph

An Octatrack-style parameter morpher for the OP-XY, built on the
[Open-XY SDK](https://github.com/etcetc000/open-xy).

New to Morph? Start with the [guide](GUIDE.md).

The morpher is a silent synth: put it on an instrument track. Its page has
two pages of four controls; pressing any knob switches between them.

| Control | Values |
| --- | --- |
| Mode | play, write A, write B: each detent of the knob moves one mode |
| Morph | 0% (all state A) to 100% (all state B); 50% is the exact midpoint |
| State A | off, 1–16 |
| State B | off, 1–16 |
| Frame (page 2) | on, off: whether the pages of tracks Morph is changing show a thin dark red frame |
| Rate (page 2) | the LFO's rate: free, 0.05 to 20 Hz, then synced to the tempo, 16 bars to 1/16 |
| Depth (page 2) | -100 to +100: how far the LFO swings the morph either side of the Morph knob; 0 is off |
| Shape (page 2) | sine, triangle, ramp up, ramp down, square, random |

**Bypass:** M4 on Morph's track switches Morph off and on (Morph's page
labels it "bypass", just above M4). Bypassed, Morph morphs, writes and
marks nothing, every track plays its own values, and its page is dimmed. The
bypass is saved with the project.

Morph's LFO moves the morph in play mode, on every page; while writing it
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

Other pages show what Morph is doing too: while you write a state, every page
has a red frame at the screen's edge, and the page of a track Morph is
changing has a thin dark red frame (unless Frame is off).

## Writing states

Choose a state number for A (or B), set the mode to write A (or write B), and
turn any encoder on another instrument track: each edit is recorded into that
state. While writing you hear that state alone, as at morph 0% (or 100%), with
your edits, whatever the morph control says: its percentage is boxed in red
until you return to play, where it carries on from
where it was. Leaving write mode puts every edited parameter back where it
was. A state holds up to 32 parameters. Only the instrument tracks (1–8) are
recorded: knobs turned on the auxiliary tracks (9–16) are not.

Writing a side whose state is off records nothing: that state's "off" is boxed
in red, and the morph plays as in play mode.

## Playing

In play mode the morpher blends every parameter state A or B sets on the other
instrument tracks. A side that is off, or a state that leaves a parameter
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

## One Morph at a time

Only one Morph can run. If more than one instrument track holds Morph, none of
them morphs: every track goes back to its own values, an unfinished write is
undone, and each Morph page shows "ONLY ONE MORPH ALLOWED" until only one is
left. States are kept, whichever track Morph is on.

## Status

The morpher needs the SDK's track control (SDK 1.5), which is not in a
released SDK yet. States are saved with the project, in the companion file
beside it (`song.oxy` beside `song.xy`; copy both when you share a project),
and come back when it loads. A project saved without them, or on stock
firmware since, starts with every state empty. There is no way to clear a
single state yet.

A possible later upgrade: Morph on an auxiliary track (track 12, CV out, chosen
with Shift + M1) instead of an instrument track, so it doesn't use up one of
the eight sound tracks. It needs more firmware hooks (silencing the CV output
while Morph holds the track), so it waits until the rest is finished.

## Why Morph doesn't use the OP-XY's LFO

For reviewers. On the OP-XY, an LFO on a synth's control is evaluated by the
audio side for voices that are sounding, and, for a track with none, only
while that track is selected. Morph makes no sound, so stock's LFO moved the
morph only while Morph's page was shown. We traced this in the firmware
(OS 1.2.0 and 1.2.1), forced the selected-track check, and held notes on
Morph's track: stock never evaluated the LFO for Morph's track while another
track was shown. So Morph hides the stock LFO page and runs its own LFO
on the morph, on its second page, computed on the GUI side every tick, where
it works whichever page is shown.

## Checks

From this repository, with the SDK checked out beside it:

```sh
python ../open-xy/sdk.py test --machine machines/morph --firmware
```

On an OP-XY, [QA.md](QA.md) lists the checks by area; running every section is
the end-to-end test.
