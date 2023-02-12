#!/usr/bin/env python3

from pygame import midi
import time
import random

delay = 0.8
velocity_range = (95,127)
#note_range = (30,100)
note_choice_maj9 = [30, 34, 37, 41, 42, 44, 46, 49, 53, 54, 56, 58, 61, 65, 68]
note_choice_9 =    [37, 41, 44, 47, 49, 51, 53, 56, 59, 61, 63, 65, 68, 71, 75]
#note_choice = [68]
notes_on = [[],[]]
current_instrument = 0
keys = [1, 4, 5]
num_notes_played = 0
note_choice = note_choice_maj9

arpeggio1 = [54, 58, 61, 65] # 54 maj 7
arpeggio2 = [54, 58, 61, 63] # 63 min 7
arpeggio3 = [54, 57, 61, 62] # 62 maj 7
arpeggio4 = [54, 56, 59, 62] # 56 min 7 b5
arpeggio_roots = [42, 39, 38, 44]
velocity_arp = [120, 90, 70, 60]
delay_arp = 0.2

sysex_msgs = [
  b'\xF0\x7D\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1F\x1E\x1D\xF7',
  b'\xF0\x7D\x10\x11\x12\x13\x14\x15\x16\xF7',
  b'\xF0\xF7'
]

def init():
  midi.init()
  print('initialized pygame midi')

def sysex(output):
  while True:
    output.write_sys_ex(midi.time(), random.choice(sysex_msgs))
    time.sleep(delay_arp * 5)

def test_control_change(output):
  global current_instrument
  while True:
    output.write_short(0xb0, 0x7f, 0x7f)
    time.sleep(delay)

def arpeggiator(output):
  global current_instrument
  while True:
    for arp, root in zip([arpeggio1, arpeggio2, arpeggio3, arpeggio4], arpeggio_roots):
      notes_on[current_instrument].append(root)
      output.note_on(root, 127)
      for note, velocity in zip(arp, velocity_arp):
        notes_on[current_instrument].append(root)
        output.note_on(note, velocity)
        time.sleep(delay_arp)
        output.note_off(note, velocity)
        del notes_on[current_instrument][1]
        time.sleep(delay_arp/2)
      output.note_off(notes_on[current_instrument][0], 64)
      del notes_on[current_instrument][0]
      current_instrument = random.randint(0,1)
      output.set_instrument(current_instrument)

def random_loop(output):
  global note_choice
  key = 1
  num_notes_played = 0
  while True:
    # after two or three are on
    if len(notes_on[current_instrument]) > random.randint(2,3):
      # turn one off
      note_idx = random.randrange(0,len(notes_on[current_instrument]))
      note = notes_on[current_instrument][note_idx]
      output.note_off(note, 64)
      print("OFF", note, 64)
      del notes_on[current_instrument][note_idx]

    if random.randint(0,1) > 0:
      time.sleep(delay)

    if len(notes_on[current_instrument]) == 0 or random.randint(0,4) > 0:
      # random note on
      #note = random.randint(*note_range)
      note = random.choice(note_choice)
      if key == 4:
        note += 5
      velocity = random.randint(*velocity_range)
      output.note_on(note, velocity)
      print("ON", note, velocity)
      notes_on[current_instrument].append(note)
      num_notes_played += 1
    if num_notes_played == 10:
      num_notes_played = 0
      key = random.choice(keys)
      if key == 1:
        note_choice = note_choice_maj9
        print("Key 1")
      elif key == 4:
        note_choice = note_choice_maj9
        print("Key 4")
      elif key == 5:
        note_choice = note_choice_9
        print("Key 5")

    time.sleep(delay)

if __name__ == '__main__':
  import sys
  #sys.argv...
  init()
  output_id = midi.get_default_output_id()
  output = midi.Output(output_id)
  output.set_instrument(current_instrument)
  time.sleep(0.01)

  try:
    test_control_change(output)
    #sysex(output)
    #random_loop(output)
    #arpeggiator(output)
  except KeyboardInterrupt:
    # turn off all notes
    output.write_short(0xb0, 0x78)
    output.write_short(0xb1, 0x78)
    #output.set_instrument(0)
    #for note in range(0, 127):
    #  output.note_off(note, 64)
    #  time.sleep(0.001)
    #output.set_instrument(1)
    #for note in range(0, 127):
    #  output.note_off(note, 64)
    #  time.sleep(0.001)
    print('')
  finally:
    print('quitting')
