# Morph

An Octatrack-style parameter morpher for the OP-XY, built on the
[Open-XY SDK](https://github.com/etcetc000/open-xy).

The morpher is a silent synth: put it on an instrument track. Its page has
two pages of four controls; pressing any knob switches between them.

| Control | Values |
| --- | --- |
| Mode | play, write A, write B: each detent of the knob moves one mode |
| Morph | 0% (all state A) to 100% (all state B); 50% is the exact midpoint |
| State A | off, 1–16 |
| State B | off, 1–16 |
| Frame (page 2) | on, off: whether the pages of tracks Morph is changing show a thin dark red frame |

The other three page 2 controls are spare, for later features.

A state is a set of parameter values on other tracks, what the Octatrack calls
a scene (the OP-XY already uses "scene" for its sequencer).

"Off" leaves every parameter as it is on that side of the crossfader. All the
controls can be sequenced and p-locked like any other track parameter.

The page shows the crossfader, the two state markers (dim when off) and, while
writing, the marker being written filled red, a red dot at the top right and
the morph's percentage boxed in red, with the crossfader's ring hidden.

Other pages show what Morph is doing too: while you write a state, every page
has a red frame at the screen's edge, and the page of a track Morph is
changing has a thin dark red frame (unless Frame is off).

## Writing states

Choose a state number for A (or B), set the mode to write A (or write B), and
turn any encoder on another instrument track: each edit is recorded into that
state. While writing you hear that state alone, as at morph 0% (or 100%), with
your edits, whatever the morph control says: its ring is hidden and its
percentage boxed in red until you return to play, where it carries on from
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

The morpher needs the SDK's track control (SDK 1.4), which is not in a
released SDK yet. States live in memory only: they are lost when the OP-XY
turns off and are shared by every project. There is no way to clear a state
yet except turning the OP-XY off. An indicator on every screen and saving states
with the project come next.

A possible later upgrade: Morph on an auxiliary track (track 12, CV out, chosen
with Shift + M1) instead of an instrument track, so it doesn't use up one of
the eight sound tracks. It needs more firmware hooks (silencing the CV output
while Morph holds the track), so it waits until the rest is finished.

## Checks

From this repository, with the SDK checked out beside it:

```sh
python ../open-xy/sdk.py test --machine machines/morph --firmware
```

On an OP-XY, [QA.md](QA.md) lists the checks by area; running every section is
the end-to-end test.
