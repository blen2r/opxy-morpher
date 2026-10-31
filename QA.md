# Testing Morphose on an OP-XY

Checks to run on a device with a firmware that includes Morphose, from top to bottom, in one project:
each section leaves the project ready for the next. To run only some sections, set up what their first
line says. Host tests (`python ../open-xy/sdk.py test --machine machines/morphose --firmware`) don't
replace these: they can't hear the sound or see the screen.

Report a failure with the section, the line and what you saw. A photo of the screen helps.

## Setup

- Install the firmware. Create a new project, **plain**: leave `cv` on 12, give track 1 a sound of its
  own, and save it. Later steps load it as a project without Morphose's data.
- Create another new project, **qa**: Morphose changes other tracks' values while it runs, so don't use a
  project you care about.
- Track 1: the stock synth **simple**, its four engine knobs at 50, and one note on step 1 lasting a
  whole bar, so it sounds all the time while the sequencer plays.
- Track 12: Morphose (Shift + M1 on track 12, choose `morphose`), mode play, morph 0.
- Press play, and leave it playing unless a step says otherwise.

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
- [ ] Nothing shows in the middle of the shape or above M4 (no "muted", no stray letter)

Then clear those p-locks on track 12, and set mode play, morph 0, both states off.

## M2, M3 and M4

- [ ] On track 12 with Morphose, M2 and M3 do nothing: no CV, envelope or filter page, not even for a
      moment. M4 opens the stock LFO page
- [ ] Select track 12 from another track's M3 or M4: Morphose's own page shows
- [ ] Other tracks' M2, M3 and M4 work as before

## Writing states

Track 1's engine knobs at 50. "State 1" will hold them all the way down, "state 2" all the way up.

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
- [ ] A parameter lock made while writing (hold a step, turn a knob) is not recorded and stays a lock;
      clear it afterwards
- [ ] Turning Morphose's own knobs is never recorded, nor are knob turns in play mode

From here on, states 1 and 2 are written, and state A = 1, state B = 2, mode play.

## Applying states

- [ ] Turning morph from 0 to 100 sweeps track 1's sound, and track 1's page shows the morphed values,
      stopped and playing
- [ ] State A = off: at morph 0 track 1 sounds as you set it. Then state A back to 1
- [ ] Turning one of track 1's morphed knobs: its page shows your value while you turn, then the morphed
      value again
- [ ] Parameters no state sets behave as without Morphose, including parameter locks shown while holding
      a step
- [ ] A parameter lock on a morphed parameter is heard only as far as the morph leans toward a side that
      leaves it alone; clear it afterwards
- [ ] Sweep the morph fast for a while: no clicks, stutter, freezes or stuck values
- [ ] The morph is never saved into the project: at morph 100, save the project and load it again, then
      choose `cv` on track 12: track 1 returns to its own sound, its knobs at their own values (50), not
      state 2's
- [ ] Choose `morphose` on 12 again: it starts at its defaults (play, morph 0, states off); set state A to
      1 and state B to 2: states 1 and 2 are still there

## Marks on every page

- [ ] While writing, every page of every track has a red frame fading inward from the screen's edge
- [ ] Frame on (page 2): the pages of tracks Morphose is changing have a thin dark red frame in play;
      frame off: no thin frame. Then frame on again
- [ ] No frame with `cv` on 12

## Presets

- [ ] Shift + track key on track 12: "preset", with 12 under it; folders in the first column, sorted by
      name, presets in the second; `morphose` holds `init`
- [ ] Knobs 2-4 scroll and choose; choosing `init` sets play, morph 0, states off, frame on
- [ ] A `cv` folder holds `init`: choosing it gives track 12 back to CV
- [ ] With `cv` on 12, choosing Morphose's `init` puts Morphose there
- [ ] Shift + track key on track 1: no aux presets among the instrument presets

Then state A = 1, state B = 2 again.

## Morphose's LFO (page 2)

Morph at 50, so the whole swing shows: near 0 or 100, the part past the end is cut off and the swing
looks shorter and faster than it is.

- [ ] Depth 0: nothing moves. Depth +50, rate 1 (Hz): the shape swings once per second and track 1's
      sound follows, on Morphose's page and on track 1's page
- [ ] Free rates: at 0.5 a full swing takes 2 seconds, at 2 half a second
- [ ] A synced rate (e.g. 1bar) with the sequencer playing: the swing lines up with the bar, the same each
      pass; stopped, it keeps the tempo
- [ ] Shape tri, rate 1bar, depth +50, a 1-bar pattern: each bar starts at 50 rising, peaks at 100 on
      beat 2 (step 5), is back at 50 on beat 3 (step 9) and at 0 on beat 4 (step 13). Depth -50: the
      other way round
- [ ] Each shape: sine, tri, up, down, sqr and rand (a new random value each cycle, spread evenly over
      the range the swing can reach: with the knob near 0 and a large depth, it doesn't keep landing on 0)
- [ ] Depth moves one per detent (-100..+100), two when turned fast
- [ ] Depth -50 reverses the swing; with the morph knob near an end, the swing stops at 0 or 100
- [ ] In wr a or wr b the LFO pauses, so you hear the state as written, and it resumes in play (turn no
      other track's knob while writing here)
- [ ] Rate, depth and shape can be p-locked; clear the locks afterwards

Then depth +50, rate 1, shape sine, morph 50: Morphose's LFO is kept on for the next sections.

## The stock LFO (M4) on track 12

- [ ] With Morphose on track 12, M4's destinations read Morphose's control names (mode, morph, state a,
      state b), not param1-4
- [ ] Morphose's LFO at depth 0 for now. M4's LFO on morph: the morph sweeps and track 1's sound follows,
      on Morphose's page and with another track selected (track 1's page, a send's page), playing or stopped
- [ ] The LFO's rate, depth and shape behave as on a stock synth; turning it off stops the sweep
- [ ] The LFO on mode or a state: it switches them as it moves (expected), and nothing freezes
- [ ] Both LFOs at once: M4's on state a (or morph) and Morphose's (page 2, depth +50) on the morph: both
      are heard, adding up, with nothing stuck when either stops

Then M4's LFO on morph, on; Morphose's LFO at depth +50.

## Patterns and songs

Track 12 holds CV or Morphose per pattern, as tracks 1-8 hold an engine per pattern. CV has no knobs to
tell its patterns apart: once another aux machine is released, put it on pattern 2 with its knobs set
differently and check that each pattern keeps its own machine and values.

Here, track 12 gets three patterns: 1 Morphose, 2 `cv`, 3 a copy of 1. A song plays scenes, so the
scenes are: scene 1 with track 12 on pattern 1, scene 2 on pattern 2, scene 3 on pattern 3.
- [ ] Add pattern 2 on track 12 and choose `cv` there. Pattern 1 still shows Morphose with its states;
      pattern 2 shows CV
- [ ] While playing, switch track 12 between patterns 1 and 2 a few times: Morphose morphs only on pattern
      1, track 1 plays its own sound on pattern 2
- [ ] Add pattern 3, copied from pattern 1: it holds Morphose with the same knobs and states
- [ ] Make scenes 1, 2 and 3 as above. A song of scene 1 then scene 2, looping, for a few loops:
      Morphose morphs only in scene 1, and on each return its states are there and both LFOs carry on
- [ ] Both LFOs at depth 0, morph 0 on pattern 1 and 100 on pattern 3. A song of scene 1 then scene 3,
      looping: each scene plays and shows its own morph from its first bar, on every pass; a song of
      scenes 1, 2 and 3: the same, with CV in scene 2
- [ ] On track 12, p-lock Morphose's morph to 50 on step 1 of pattern 3 (its morph knob stays at 100). In
      the song of scenes 1 and 3, scene 3 starts at 50, on every pass, not at 100. Then clear the lock
- [ ] Track 12's own settings follow the scene: M4's LFO amount at 0 on pattern 1 and at max on pattern 3,
      with a different rate or shape too. Start the song of scenes 1 and 3 from pattern 3: scene 1 plays and
      shows its own LFO (none) from the start, scene 3 its own, on every pass

Then leave the song and play scene 1: pattern 1 at morph 50, both LFOs back on (Morphose's at depth +50,
M4's on morph).

## Muting (switches Morphose off)

- [ ] At 50 between states 1 and 2, mute track 12 (hold aux, press track 12's key): track 1 goes back to
      its own sound, "muted" shows in the middle of the shape (the whole word), the shape and markers dim,
      no thin frame shows, and neither LFO moves the shape
- [ ] While muted, wr a records nothing and shows no red frame; back to play
- [ ] Unmute: everything comes back as it was
- [ ] The same from the mixer
- [ ] Track 12 muted in scene 3 only: switching between scenes 1 and 3 (and the song of scenes 1 and 3)
      switches Morphose on and off

Leave track 12 muted in scene 3 only, and play scene 1.

## Track 12 and CV

Needs something on CV/gate out, or CV out into the line input, listening on track 13 (input level low):
each change of CV or gate is a click. Skip this section without one.
- [ ] With `cv` on 12 (pattern 2), notes on track 12 click; with Morphose there (pattern 1), no clicks, on
      notes, on play / stop or on fast key presses, and turning Morphose's knobs makes none either
- [ ] While playing, switch between patterns 1 and 2, and play the song of scenes 1 and 2: CV is silent on
      pattern 1 and plays on pattern 2, with no stray gate or click at the switch
- [ ] On pattern 1, choose `cv`: the clicks come back at once (no silent second). Then choose `morphose`
      again, state A = 1, state B = 2, morph 50

## Saving and loading

- [ ] Save **qa**, power off and on, load it: states 1 and 2 still morph, M4's LFO is on and sweeping,
      pattern 1 holds Morphose and pattern 2 `cv`, track 12 is muted in scene 3 only, and Morphose's
      depth and rate are as you left them
- [ ] Write a state 3 (state A = 3, wr a, turn track 1's knob 1, back to play, state A = 1), wait a few
      seconds and power off without saving, while Morphose's page is shown: the OP-XY starts normally,
      Morphose is on track 12 and state 3 is there
- [ ] Load **plain** (no states, `cv` on 12): track 12 is `cv`, track 1 has its own sound, no frames;
      put Morphose on 12: every state is empty. Load **qa** again: its states are back
- [ ] While playing, load **plain** and then **qa** again: the states come back, nothing freezes, no
      stray values on track 1

## Writing across patterns and presets

In **qa**, on pattern 1. These change track 1's values: run them after everything above.
- [ ] Set track 1's knob 1 to 20 in pattern 1 and 80 in pattern 2 (pattern 2 on track 12: choose
      `morphose` there too). On pattern 1, state A = 4, mode wr a, turn track 1's knob 1 to 50, switch to
      pattern 2, then mode to play: pattern 2's knob is still 80; back on pattern 1, it is 20, and with
      state A = 4 at morph 0 you hear 50
- [ ] State A = 5, mode wr a, turn one of track 1's knobs, then load a stock preset on track 1 (Shift +
      track 1 key), turn a knob again, mode to play: track 1 has the preset's values, not those from
      before writing

## Loading while writing, and a new project

- [ ] Mode wr a (red frame), then load **plain**: it opens with its own values, no red frame, none of
      **qa**'s states. Nothing freezes
- [ ] Load **qa**, mode wr a, then create a new project: track 12 is `cv`; put Morphose there and choose
      states 1 and 2: both are empty (nothing from **qa**)

## After the pass

These need another setup.
- [ ] Over MTP: **qa**'s folder has `qa.oxy` beside `qa.xy`. Copy a preset into
      `aux_presets/<folder>/`: it lists and works; with `cv` on 12, choosing it puts Morphose there
- [ ] On a firmware without Morphose, open **qa**: track 12 is `cv` and works as CV (notes click on its
      outputs, as in "Track 12 and CV")

## Known limits

Not failures, but worth confirming they haven't changed:

- A parameter lock holds until the pattern loops, not just for its step (stock behaviour).
- Only track 12 can hold Morphose, so the "ONLY ONE MORPHOSE" warning (two tracks holding it) can't be
  shown today; check it if another aux track can ever take machines.
