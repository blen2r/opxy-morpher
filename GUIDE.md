# morph guide

morph blends the sound of your other tracks between two snapshots of their knobs. set up two
states, then sweep the morph knob from one to the other, by hand, with parameter locks, or live.

## 1. getting started

### 1.1 put morph on a track

morph lives on an instrument track (1–8), like a synth. it makes no sound of its own.

select an empty track, hold shift and press M1 to open the engine list, then pick morph.

### 1.2 one morph at a time

only one morph can run. if two tracks hold morph, neither does anything: your tracks go back to
their own sound and both morph pages show "ONLY ONE MORPH ALLOWED". change one of them to another
engine and the other carries on, with its states intact.

## 2. the page

morph has two pages of four knobs. press any knob to switch pages.

### 2.1 page 1

- knob 1, mode: play, wr a (write state a) or wr b (write state b).
- knob 2, morph: 0% is all state a, 100% is all state b, 50% is halfway.
- knob 3, state a: off, or a state from 1 to 16.
- knob 4, state b: off, or a state from 1 to 16.

the shape in the middle is the morph: a circle at 0% (state a), a square at 100% (state b), and
in between as you turn. the crossfader under it has a tick every 5%. the circle on the left and
the square on the right stand for states a and b, and are dim when their state is off.

### 2.2 page 2

- knob 1, frame: on or off. with frame on, the pages of tracks morph is changing have a thin dark
  red frame.
- knobs 2–4 are spare, for later.

## 3. writing states

a state remembers knob values on your other instrument tracks. each project has 16.

### 3.1 write a state

turn state a to the number you want, then turn mode to wr a. now turn knobs on any other
instrument track, on any page: engine, filter, envelope and so on. each knob you turn is recorded
into the state.

while you write:

- you hear the state alone, as at morph 0%, whatever the morph knob says.
- every page has a red frame, so you always know you are recording.
- on morph's page the shape shows the state you are writing, in red, its marker is red, and the
  morph percentage is boxed in red.

turn mode back to play when you are done. every knob you turned goes back to where it was, and the
morph carries on from where it was.

write state b the same way with wr b. you hear it as at morph 100%.

### 3.2 what is and isn't recorded

- a state holds up to 32 knobs.
- only instrument tracks 1–8 are recorded, not the auxiliary tracks.
- turning knobs in play mode is never recorded.
- parameter locks are never recorded: hold a step and turn a knob while writing, and it stays a
  lock on its step.
- writing a side whose state is off records nothing. its "off" is boxed in red so you can see why.

## 4. morphing

### 4.1 sweep between states

in play mode, turn morph from 0% to 100%. every knob state a or b recorded moves from a's value to
b's. knobs neither state recorded are left alone.

a side that is off, or a state that doesn't hold a knob, uses the track's own value for that knob.
so with state b off, morph 100% is your tracks as you set them.

### 4.2 your own knobs while morphing

a knob a state controls follows the morph, not your hand. turn it and its page shows your value
for a moment, then the morphed value again. your value is kept and comes back when morph stops
controlling that knob.

a parameter lock on a morphed knob is heard only as far as the morph leans toward a side that
doesn't hold that knob.

### 4.3 sequence the morph

morph's knobs work like any other track's: hold a step and turn morph to lock it on that step.
lock state a or state b on different steps to switch states as the pattern plays.

## 5. saving

states are saved with the project. morph keeps them in a small file beside the project
(song.oxy beside song.xy): copy both when you share a project.

load a project and its states come back. a project saved without them starts with all 16 empty.

## 6. limits

- an lfo on the morph knob moves the morph only while morph's page is shown.
- there is no way to clear a single state yet.
