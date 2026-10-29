# Testing Morphose on an OP-XY

Checks to run on a device with a firmware that includes Morphose. Each section stands on its own:
run the sections a change touches, or every section for a full end-to-end pass before a
release. Host tests (`python ../open-xy/sdk.py test --machine machines/morphose --firmware`) don't
replace these: they can't hear the sound or see the screen.

Report a failure with the section, the line and what you saw. A photo of the screen helps.

## Setup

- Use a scratch project: Morphose changes other tracks' values while it runs.
- Track 1: the stock synth **simple**, its four engine knobs at 50, and one note on step 1 lasting a
  whole bar, so it sounds all the time while the sequencer plays.
- Track 12: Morphose (Shift + M1 on track 12, choose `morphose`), mode play, morph 0.
- Press play. "State 1" is track 1's engine knobs all the way down, "state 2" all the way up: write
  them as in [Writing states](#writing-states) before the sections that use them.

## Firmware basics

- [ ] The OP-XY boots, and stock engines, effects, the sequencer and project save/load work
- [ ] Morphose is not in the engine list of tracks 1-8 (Shift + M1 there)
- [ ] Shift + M1 on track 12: a list titled "engine" with `cv` and `morphose`, the current one highlighted;
      M2 closes it without changing anything
- [ ] Shift + M1 on tracks 9, 10, 11, 13 and 14 does nothing new; on 15 and 16 it opens the effect list,
      as Shift + track key does there

## Morphose's page

- [ ] Morph 0 shows a circle, 100 a square with sharp corners; in between the shape blends smoothly
      while turning. The header reads 0 to 100, 50 in the middle
- [ ] The markers: a circle (A) on the left and a square (B) on the right, letters centred, dim while
      their state is off
- [ ] Mode reads play / wr a / wr b, and the states off, 1 to 16: one choice per detent, both directions
- [ ] p-lock the morph knob on a few steps and play: the shape follows; sweeping fast causes no flicker
      or slowdown
- [ ] p-lock the mode on a step: the shown name follows the lock during playback
- [ ] Both pages show in the header: page 1 (mode, morph, state a, state b) on top, page 2 (frame, rate,
      depth, shape) below; the row the knobs edit is large and bright on the stepped bar, the other small
      and grey
- [ ] Pressing any knob switches the knobs to the other page: the rows swap size smoothly, page 1 stays on
      top, and a second press mid-way reverses
- [ ] A small grey "bypass" label sits at the bottom right, above M4

## Writing states

Start from the [setup](#setup): track 1's knobs at 50, the sequencer playing.

- [ ] **State 1:** on Morphose's page, set morph to 100 (state B stays off), state A to 1, then mode to
      wr a. On track 1's engine page, turn its four knobs all the way down: you hear each turn
- [ ] While writing, you hear state A alone, as at morph 0, whatever the morph knob says; Morphose's page
      shows a red circle, a red A marker, a red dot at the top right and the morph value boxed in red
- [ ] Mode to play: at morph 100 with state B off, track 1 plays its own values again, its knobs back at 50
- [ ] **State 2:** set morph to 0, state B to 2, then mode to wr b. Turn track 1's four engine knobs all
      the way up, and also one knob on its filter page, one on its envelope page and one on track 3: you
      hear each turn, and Morphose's page shows a red square
- [ ] Mode to play, at morph 0: track 1's engine knobs show state 1 (all the way down), and the filter,
      envelope and track 3 knobs are back where they were. Turning morph to 100 moves to state 2 on every
      track and page you edited
- [ ] Writing a side whose state is off: "off" is boxed in red and nothing is recorded
- [ ] A parameter lock made while writing (hold a step, turn a knob) is not recorded and stays a lock
- [ ] Turning Morphose's own knobs is never recorded, nor are knob turns in play mode

## Applying states

- [ ] With state A = 1 and state B = 2 written, turning morph from 0 to 100 sweeps track 1's sound, and
      track 1's page shows the morphed values, stopped and playing
- [ ] State A = off: at morph 0 track 1 sounds as you set it
- [ ] Turning one of track 1's morphed knobs: its page shows your value while you turn, then the morphed
      value again
- [ ] Parameters no state sets behave as without Morphose, including parameter locks shown while holding
      a step
- [ ] A parameter lock on a morphed parameter is heard only as far as the morph leans toward a side that
      leaves it alone
- [ ] Sweep the morph fast for a while: no clicks, stutter, freezes or stuck values
- [ ] Choose `cv` on track 12: track 1 returns to its own sound
- [ ] The morph is never saved into the project: at morph 100, save the project and load it again, then
      choose `cv` on track 12: track 1's knobs are at their own values (50), not
      state 2's

## Marks on every page

- [ ] While writing, every page of every track has a red frame fading inward from the screen's edge
- [ ] Frame on (page 2): the pages of tracks Morphose is changing have a thin dark red frame in play;
      frame off: no thin frame
- [ ] No frame without Morphose, while it is bypassed, or while two tracks hold it

## Track 12 and CV

Needs something on CV/gate out, or CV out into the line input, listening on track 13 (input level low):
each change of CV or gate is a click.
- [ ] With `cv` on 12, notes on track 12 click; with Morphose there, no clicks, on notes, on play / stop
      or on fast key presses, and turning Morphose's knobs makes none either
- [ ] Choose `cv` again: the clicks come back, and CV's four knobs are where they were before Morphose

## Presets

- [ ] Shift + track key on track 12: "preset", with 12 under it; folders in the first column, sorted by
      name, presets in the second; `morphose` holds `init`
- [ ] Knobs 2-4 scroll and choose; choosing `init` sets play, morph 0, states off, frame on
- [ ] A preset in `aux_presets/<folder>/` (copied over MTP) lists and works too; with `cv` on 12,
      choosing a Morphose preset puts Morphose there
- [ ] Shift + track key on track 1: no aux presets among the instrument presets

## Saving states

- [ ] Write states 1 and 2, save the project, power off and on, load it: morph 0 to 100 still moves
      between them
- [ ] The project folder has `<name>.oxy` beside `<name>.xy` (over MTP)
- [ ] Load a project saved before it had states: every state is empty; load the first project again:
      its states are back
- [ ] Write a state, wait a few seconds and power off without saving: after power on, the state is still
      there, and Morphose is on track 12
- [ ] Power off while Morphose's page is shown, without saving: the OP-XY starts normally
- [ ] With states written, create a new project: track 12 is `cv`; put Morphose there and choose states
      1 and 2: both are empty (nothing from the previous project)
- [ ] A project saved with `cv` on 12 loads with `cv`

## M2, M3 and M4

- [ ] On track 12 with Morphose, M2 does nothing and M3 leaves Morphose's page as it is (no CV screen)
- [ ] Select track 12 from another track's M3 or M4: Morphose's own page shows
- [ ] Other tracks' M2, M3 and M4 work as before

## Morphose's LFO (page 2)

- [ ] Depth 0: nothing moves. Depth +50, rate 1 (Hz): the shape swings once per second and track 1's
      sound follows, on Morphose's page and on track 1's page
- [ ] Free rates: at 0.5 a full swing takes 2 seconds, at 2 half a second
- [ ] A synced rate (e.g. 1bar) with the sequencer playing: the swing lines up with the bar, the same each
      pass; stopped, it keeps the tempo
- [ ] Each shape: sine, tri, up, down, sqr and rand (a new random value each cycle, spread evenly over
      the range the swing can reach: with the knob near 0 and a large depth, it doesn't keep landing on 0)
- [ ] Depth -50 reverses the swing; with the morph knob near an end, the swing stops at 0 or 100
- [ ] In wr a or wr b the LFO pauses, so you hear the state as written, and it resumes in play
- [ ] Rate, depth and shape can be p-locked

## Bypass (M4)

- [ ] At 50 between states 1 and 2, press M4 on Morphose's track: track 1 goes back to its own sound,
      the label reads "bypassed", the shape and markers dim, no thin frame shows, and the LFO stops
- [ ] While bypassed, wr a records nothing and shows no red frame
- [ ] Press M4 again: everything comes back as it was
- [ ] Bypass, save, power off and on, load: still bypassed
- [ ] On another track's M4 (LFO page), select Morphose's track: Morphose's page shows, and the bypass
      doesn't change

## Known limits

Not failures, but worth confirming they haven't changed:

- A parameter lock holds until the pattern loops, not just for its step (stock behaviour).
