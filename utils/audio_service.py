#!/usr/bin/env python3

from array import array
import numpy
import mmap
import pygame
import time
import multiprocessing as mp
import copy

# 4096 frames, 2 samples per frame, each sample is a float
chunk_size = 2048
max_millis = 50
chunk_size = 1024
max_millis = 23
#chunk_size = 512
#max_millis = 12
#chunk_size = 256
#max_millis = 6
n = chunk_size * 2 * 32

def play_loop(q, done):
  init()
  while not done.value:
    if not q.empty():
      while pygame.mixer.get_busy():
        continue
      chunk = q.get()
      sound = pygame.mixer.Sound(chunk)
      sound.play(maxtime=max_millis)

def read_loop(q):
  # TODO truncate ./DEADBEEF to the correct size
  with open("./DEADBEEF", "r+b") as f:
    with mmap.mmap(f.fileno(), 0) as mm:
      print('Listening')
      mm[:4] = b'0000'
      while True:
        head = array('b', mm[:4])
        if bytes(head) == b'AAAA':
          chunk1 = numpy.frombuffer(mm[4:n], dtype=float).reshape((chunk_size>>1,2))
          chunk2 = copy.deepcopy(chunk1)
          mm[:4] = b'0000'
          q.put(chunk2)
          while not q.empty():
            while pygame.mixer.get_busy():
              continue
            chunk = q.get()
            sound = pygame.mixer.Sound(chunk)
            sound.play(maxtime=max_millis)

def init():
  pygame.mixer.pre_init(frequency=48000, size=32, channels=2, buffer=chunk_size)
  pygame.mixer.init()
  pygame.init()
  print('initialized pygame')

if __name__ == '__main__':
  import sys
  #sys.argv...
  init()
  with mp.Manager() as manager:
    sound_queue = mp.Queue()
    done = manager.Value('d', False)
    #p = mp.Process(target=play_loop, args=(sound_queue, done))

    try:
      #p.start()
      read_loop(sound_queue)
    except KeyboardInterrupt:
      done.value = True
      print('')
    finally:
      #p.join()
      print('quitting')
