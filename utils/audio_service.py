#!/usr/bin/env python3

from array import array
import numpy
import mmap
import pygame
import time

# 4096 frames, 2 samples per frame, each sample is a float
chunk_size = 1024
n = chunk_size * 2 * 32

def loop():
  with open("./DEADBEEF", "r+b") as f:
    with mmap.mmap(f.fileno(), 0) as mm:
      print('Listening')
      mm[:4] = b'0000'
      while True:
        head = array('b', mm[:4])
        if bytes(head) == b'AAAA':
          chunk = numpy.frombuffer(mm[4:n], dtype=float).reshape((chunk_size>>1,2)) #array('f', mm[4:n])
          sound = pygame.mixer.Sound(chunk)
          mm[:4] = b'0000'
          while pygame.mixer.get_busy():
            time.sleep(0.01)
          sound.play()
        else:
          time.sleep(0.02)

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
