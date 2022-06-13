#!/usr/bin/env python3

from array import array
import numpy
import mmap
import pygame
import time
import sys

# 4096 frames, 2 samples per frame, each sample is a float
chunk_size = 2048
max_millis = 50
chunk_size = 1024
max_millis = 23
chunk_size = 512
max_millis = 12
chunk_size = 256
#max_millis = 6
n = chunk_size * 2 * 32
sound_queue = []

def loop():
  # TODO truncate ./DEADBEEF to the correct size
  with open("./DEADBEEF", "r+b") as f:
    with mmap.mmap(f.fileno(), 0) as mm:
      print('Listening')
      mm[:4] = b'0000'
      while True:
        head = array('b', mm[:4])
        if bytes(head) == b'AAAA':
          chunk = numpy.frombuffer(mm[4:n], dtype=float).reshape((chunk_size>>1,2))
          sound_queue.append(pygame.mixer.Sound(chunk))
          mm[:4] = b'0000'
        # TODO separate into another thread
        if len(sound_queue) > 0:
          while len(sound_queue) > 0:
            if not pygame.mixer.get_busy(): #determined by sound length in samples
              sound = sound_queue.pop()
              sound.play(maxtime=max_millis) #maxtime doesnt determine mixer.get_busy()

def init():
  pygame.mixer.pre_init(frequency=48000, size=32, channels=2, buffer=chunk_size)
  pygame.mixer.init()
  pygame.init()
  print('initialized pygame')

if __name__ == '__main__':
  import sys
  #sys.argv...
  init()
  try:
    loop()
  except KeyboardInterrupt:
    print('')
  finally:
    print('quitting')
