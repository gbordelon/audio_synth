#!/usr/bin/env python3

from pygame import midi
import time
import random

delay = 0.8
velocity_range = (95,127)
#note_range = (30,100)
note_choice = [30, 34, 37, 41, 42, 44, 46, 49, 53, 54, 56, 58, 61, 65, 68]
#note_choice = [68]
notes_on = []

arpeggio1 = [54, 58, 61, 65] # 54 maj 7
arpeggio2 = [54, 58, 61, 63] # 63 min 7
arpeggio3 = [54, 57, 61, 62] # 62 maj 7
arpeggio4 = [54, 56, 59, 62] # 56 min 7 b5
arpeggio_roots = [42, 39, 38, 44]
velocity_arp = [120, 90, 70, 60]
delay_arp = 0.2

def init():
  midi.init()
  print('initialized pygame midi')

def arpeggiator(output):
  while True:
    for arp, root in zip([arpeggio1, arpeggio2, arpeggio3, arpeggio4], arpeggio_roots):
      output.note_on(root, 120)
      notes_on.append(root)
      for note, velocity in zip(arp, velocity_arp):
        output.note_on(note, velocity)
        notes_on.append(note)
        time.sleep(delay_arp)
        output.note_off(note, velocity)
        del notes_on[1]
        time.sleep(delay_arp/2)
      output.note_off(notes_on[0], 64)
      del notes_on[0]

def random_loop(output):
  while True:
    # after two or three are on
    if len(notes_on) > random.randint(2,3):
      # turn one off
      note_idx = random.randrange(0,len(notes_on))
      note = notes_on[note_idx]
      output.note_off(note, 64)
      print("OFF", note, 64)
      del notes_on[note_idx]

    if random.randint(0,1) > 0:
      time.sleep(delay)

    if len(notes_on) == 0 or random.randint(0,4) > 0:
      # random note on
      #note = random.randint(*note_range)
      note = random.choice(note_choice)
      velocity = random.randint(*velocity_range)
      output.note_on(note, velocity)
      print("ON", note, velocity)
      notes_on.append(note)

    time.sleep(delay)

if __name__ == '__main__':
  import sys
  #sys.argv...
  init()
  output_id = midi.get_default_output_id()
  output = midi.Output(output_id)

  try:
    arpeggiator(output)
  except KeyboardInterrupt:
    # turn off all notes
    for note in notes_on:
      output.note_off(note, 64)
    print('')
  finally:
    print('quitting')
