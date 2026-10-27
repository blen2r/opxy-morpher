# Testing Morph on an OP-XY

Checks to run on a device with a firmware that includes Morph. Each section stands on its own:
run the sections a change touches, or every section for a full end-to-end pass before a
release. Host tests (`python ../open-xy/sdk.py test --machine machines/morph --firmware`) don't
replace these: they can't hear the sound or see the screen.

Report a failure with the section, the line and what you saw. A photo of the screen helps.

## Setup

- Use a scratch project: Morph changes other tracks' values while it runs.
- Track 1: a stock synth (e.g. prism or simple) playing a sequence or a held note.
- Track 2: Morph, mode play.
- Unless a check says otherwise, "state 1" means track 1's four engine knobs all the way down, and
  "state 2" all the way up. Write them as in [Writing states](#writing-states).

## Firmware basics

- [ ] The OP-XY boots, and stock engines, effects, the sequencer and project save/load work
- [ ] Morph is in the engine list (Shift + M1 on an instrument track)
- [ ] Playing notes on Morph's track makes no sound

## Morph's page

- [ ] The page shows a crossfader of 21 dots with a ring, an A circle on the left and a B circle on the right
- [ ] Morph reads 0% to 100%, 50% in the middle; at every 5% the ring sits exactly on a dot
- [ ] Mode reads play / wr a / wr b and states read off, 1 … 16, one choice per knob detent, both directions
- [ ] Pressing a knob does nothing
- [ ] A circle is dim while its state is off
- [ ] p-lock the morph knob on a few steps and play: the ring follows
- [ ] p-lock the mode on a step: the shown name follows the lock during playback

## Applying states

- [ ] State A = 1, state B = 2: turning morph from 0% to 100% sweeps track 1's sound
- [ ] Track 1's page shows the morphed values, with the sequencer stopped and playing
- [ ] State A = off: at morph 0% track 1 sounds as you set it
- [ ] Turning one of track 1's morphed knobs: its page shows your value while you turn, then the morphed
      value about half a second after you stop
- [ ] Parameters no state sets behave as without Morph, including parameter locks shown while holding a step
- [ ] A parameter lock on a morphed parameter is heard only as far as the morph leans toward a side that
      leaves it alone (see "What wins" in the README)
- [ ] Sweep the morph fast for a while: no clicks, stutter, freezes or stuck values
- [ ] Change track 2's engine away from Morph: track 1 returns to its own sound
- [ ] Save the project at morph 100%, reload it with Morph removed: track 1 has its own values

## Writing states

- [ ] State A = 1, mode wr a: turn knobs on track 1 (engine page and others, e.g. filter, envelope) and on
      track 3. You hear the changes
- [ ] While writing, the A circle is filled red, a red dot shows at the top right, the morph's % is boxed
      in red and the crossfader's ring is hidden
- [ ] You hear state A alone, as at morph 0%, whatever the morph knob says
- [ ] Back to play: every knob you turned returns to where it was, the ring comes back and the morph is
      where it was
- [ ] Same with state B = 2 and wr b (heard as at morph 100%)
- [ ] Morph 0% → 100% now moves between the two states on every track and page you edited
- [ ] Write mode with that side's state off: "off" is boxed in red, nothing is recorded, and the morph
      plays as in play mode
- [ ] A parameter lock made while writing (hold a step, turn a knob) is not recorded and stays a lock
- [ ] Turning Morph's own knobs is never recorded
- [ ] Knob turns in play mode are not recorded

## One Morph at a time

- [ ] Morph on track 2 at 50% between states 1 and 2, then put Morph on track 4 too: track 1 returns to
      its own sound, and both Morph pages show "ONLY ONE MORPH ALLOWED" / "remove the extra Morphs"
- [ ] Change track 4 away from Morph: track 2's page is back and the morph applies again, states intact
- [ ] In wr a on track 2, add a second Morph: the edits made while writing are undone

## Saving states

- [ ] Write states 1 and 2, save the project, power off and on, load it: morph 0% → 100% still moves
      between them
- [ ] The project folder has `<name>.oxy` beside `<name>.xy` (over MTP)
- [ ] Load a project saved before states existed: every state is empty
- [ ] Load the first project again: its states are back
- [ ] Write a state and power off without saving: after power on, the state is still there

## Known limits

Not failures, but worth confirming they haven't changed:

- An LFO on Morph's morph knob moves the morph only while Morph's page is shown.
- A parameter lock holds until the pattern loops, not just for its step (stock behaviour).
