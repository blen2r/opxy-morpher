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
- Track 2: Morphose, mode play, morph 0.
- Press play. "State 1" is track 1's engine knobs all the way down, "state 2" all the way up: write
  them as in [Writing states](#writing-states) before the sections that use them.

## Firmware basics

- [ ] The OP-XY boots, and stock engines, effects, the sequencer and project save/load work
- [ ] Morphose is in the engine list (Shift + M1 on an instrument track)
- [ ] Playing notes on Morphose's track makes no sound

## Morphose's page

- [ ] Morph 0 shows a circle, 100 a square with sharp corners; in between the shape blends smoothly
      while turning. The header reads 0 to 100, 50 in the middle
- [ ] The markers: a circle (A) on the left and a square (B) on the right, letters centred, dim while
      their state is off
- [ ] Mode reads play / wr a / wr b, and the states off, 1 to 16: one choice per detent, both directions
- [ ] p-lock the morph knob on a few steps and play: the shape follows; sweeping fast causes no flicker
      or slowdown
- [ ] p-lock the mode on a step: the shown name follows the lock during playback
- [ ] Pressing any knob switches to page 2 (frame, rate, depth, shape) and back
- [ ] A small grey "bypass" label sits at the bottom right, above M4

## Writing states

Start from the [setup](#setup): track 1's knobs at 50, the sequencer playing.

- [ ] **State 1:** on Morphose's page, set state A to 1, then mode to wr a. On track 1's engine page, turn
      its four knobs all the way down: you hear each turn
- [ ] While writing, you hear state A alone, as at morph 0, whatever the morph knob says; Morphose's page
      shows a red circle, a red A marker, a red dot at the top right and the morph value boxed in red
- [ ] Back on Morphose's page, mode to play: track 1's knobs return to 50
- [ ] **State 2:** set state B to 2, then mode to wr b. Turn track 1's four engine knobs all the way up,
      and also one knob on its filter page, one on its envelope page and one on track 3: you hear each
      turn, and Morphose's page shows a red square
- [ ] Mode to play: every knob you turned returns to where it was, and turning morph from 0 to 100 moves
      between the two states on every track and page you edited
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
- [ ] Change track 2's engine away from Morphose: track 1 returns to its own sound
- [ ] Save the project at morph 100, reload it with Morphose removed: track 1 has its own values

## Marks on every page

- [ ] While writing, every page of every track has a red frame fading inward from the screen's edge
- [ ] Frame on (page 2): the pages of tracks Morphose is changing have a thin dark red frame in play;
      frame off: no thin frame
- [ ] No frame without Morphose, while it is bypassed, or while two tracks hold it

## One Morphose at a time

- [ ] Morphose on track 2 at 50 between states 1 and 2, then put Morphose on track 4 too: track 1 returns
      to its own sound, and both Morphose pages show "ONLY ONE MORPHOSE" / "remove the extra copies",
      centred, also after switching between the two tracks; nothing of it shows otherwise
- [ ] Change track 4 away from Morphose: track 2's page is back and the morph applies again, states intact
- [ ] In wr a on track 2, add a second Morphose: the edits made while writing are undone

## Saving states

- [ ] Write states 1 and 2, save the project, power off and on, load it: morph 0 to 100 still moves
      between them
- [ ] The project folder has `<name>.oxy` beside `<name>.xy` (over MTP)
- [ ] Load a project saved before it had states: every state is empty; load the first project again:
      its states are back
- [ ] Write a state and power off without saving: after power on, the state is still there

## M2, M3 and M4

- [ ] On Morphose's track, M2 and M3 leave the page as it is
- [ ] On M2 of another track, select Morphose's track: Morphose's own page shows, not its envelope
- [ ] Other tracks' M2, M3 and M4 work as before
- [ ] A stock synth with its M4 LFO on, moving a knob: change its engine to Morphose: nothing keeps
      moving. Change it back to the stock synth: its LFO is off, and turns on again as usual

## Morphose's LFO (page 2)

- [ ] Depth 0: nothing moves. Depth +50, rate 1 (Hz): the shape swings once per second and track 1's
      sound follows, on Morphose's page and on track 1's page
- [ ] Free rates: at 0.5 a full swing takes 2 seconds, at 2 half a second
- [ ] A synced rate (e.g. 1bar) with the sequencer playing: the swing lines up with the bar, the same each
      pass; stopped, it keeps the tempo
- [ ] Each shape: sine, tri, up, down, sqr and rand (a new random value each cycle)
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

- Pressing M2 or M3 on Morphose's track may flash the stock page for one frame before the page comes
  back.
- A parameter lock holds until the pattern loops, not just for its step (stock behaviour).
