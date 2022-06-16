#!/usr/bin/env python3

from pygame import midi
import time
import random

delay = 0.8
velocity_range = (40,120)
note_range = (30,100)
notes_on = []

def init():
  midi.init()
  print('initialized pygame midi')

def loop(output):
  while True:
    # after two or three are on
    if len(notes_on) > random.randint(2,3):
      # turn one off
      note_idx = random.randrange(0,len(notes_on))
      note = notes_on[note_idx]
      output.note_off(note, 64)
      del notes_on[note_idx]

    if random.randint(0,1) > 0:
      time.sleep(delay)

    if len(notes_on) == 0 or random.randint(0,4) > 0:
      # random note on
      note = random.randint(*note_range)
      output.note_on(note, random.randint(*velocity_range))
      notes_on.append(note)

    time.sleep(delay)

if __name__ == '__main__':
  import sys
  #sys.argv...
  init()
  output_id = midi.get_default_output_id()
  output = midi.Output(output_id)

  try:
    loop(output)
  except KeyboardInterrupt:
    # turn off all notes
    for note in notes_on:
      output.note_off(note, 64)
    print('')
  finally:
    print('quitting')
