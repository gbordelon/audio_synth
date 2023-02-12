#include <portmidi.h>
#include <pmutil.h>
#include <porttime.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"
#include "midi.h"

bool active = false;     /* set when midi_in is ready for reading */
bool in_sysex = false;   /* we are reading a sysex message */

PmQueue *midi_to_main;
PmStream *midi_in;      /* midi input */

/*
 * this function should differentiate between sysex and everything else
 * if sysex, build up the sysex message then enqueue
 * else, enqueue
 */
static void
handle_midi_in(PmMessage data)
{
  int command;    /* the current command */
  int chan;   /* the midi channel of the current event */
  int len;    /* used to get constant field width */
  static size_t sysex_msg_idx = 0;
  static my_midi_data midi_data = { 0 };

  command = Pm_MessageStatus(data) & MIDI_CODE_MASK;
  chan = Pm_MessageStatus(data) & MIDI_CHN_MASK;

  if (in_sysex || Pm_MessageStatus(data) == MIDI_SYSEX) { // sysex
    int i;
    PmMessage data_copy = data;
    in_sysex = true;
    if (sysex_msg_idx < (sysex_max >> 2)) {
      midi_data.u.data[sysex_msg_idx] = data;
    }
    /* look for MIDI_EOX in first 3 bytes
     * if realtime messages are embedded in sysex message, they will
     * be printed as if they are part of the sysex message
     */
    for (i = 0; (i < 4) && ((data_copy & 0xFF) != MIDI_EOX); i++) {
      data_copy >>= 8;
    }

    if (sysex_msg_idx < (sysex_max >> 2)) {
      midi_data.sysex_size += i;
    }
    sysex_msg_idx++;

    if (i < 4) {
      in_sysex = false;
      if (midi_data.sysex_size < sysex_max) {
        midi_data.sysex_size++; /* include the EOX byte in output */
      }
      Pm_Enqueue(midi_to_main, &midi_data);
      memset(&midi_data, 0, sizeof(struct my_midi_st));
      sysex_msg_idx = 0;
    } else if (sysex_msg_idx >= (sysex_max >> 2)) {
      // sysex message is larger than we can read...
      // TODO realloc buffer
    }
  } else { // everything else
    midi_data.u.data[sysex_msg_idx] = data;
    Pm_Enqueue(midi_to_main, &midi_data);
  }/* else if (command == MIDI_ON_NOTE) {
    midi_data.u.data[sysex_msg_idx] = data;
    Pm_Enqueue(midi_to_main, &midi_data);
  } else if (command == MIDI_OFF_NOTE) {
    midi_data.u.data[sysex_msg_idx] = data;
    Pm_Enqueue(midi_to_main, &midi_data);
  } else if (command == MIDI_CH_PROGRAM) {
     // TODO this should change which instrument is set for gynth[0] or gsynth[1], depending on chan    midi_data.u.data[sysex_msg_idx] = data;
    Pm_Enqueue(midi_to_main, &midi_data);
  } else if (command == MIDI_CTRL) {
    // TODO handle control signals for controls 0 thru 120
    // controls 121 (MIDI_RESET_CONTROLLER) to 127 are channel mode messages. //
    if (Pm_MessageData1(data) < MIDI_ALL_SOUND_OFF) {
    } else {
      switch (Pm_MessageData1(data)) {
        case MIDI_ALL_SOUND_OFF:
          // TODO turn off all monovoices for all channels
          break;
        case MIDI_RESET_CONTROLLERS:
          // TODO default settings for a channel
          break;
        case MIDI_LOCAL:
          // TODO unused
          break;
        case MIDI_ALL_OFF:
          // TODO turn off all monovoices for a channel
          break;
        case MIDI_OMNI_OFF:
          // TODO set a channel to accept only signals from its channel
          break;
        case MIDI_OMNI_ON:
          // TODO set a channel to accept signals from all channels
          break;
        case MIDI_MONO_ON:
          if (Pm_MessageData2(data)) {
            // TODO mono for specified channel
          } else {
            // TODO mono for all channels
          }
          break;
        case MIDI_POLY_ON:
          // TODO allow polyphony for a channel
          break;
      }
    }
  } else if (command == MIDI_POLY_TOUCH && bender) {
    if (verbose) {
      // TODO allow key pressure to control an aspect of a channel's monovoice
      //printf("P.Touch Chan %2d Key %2d ", chan, Pm_MessageData1(data));
      //len = put_pitch(Pm_MessageData1(data));
      //printf(val_format + len, Pm_MessageData2(data));
    }
  } else if (command == MIDI_TOUCH && bender) {
    // TODO allow key pressure to control an aspect of a channel
    //showbytes(data, 2, verbose);
    if (verbose) {
      //printf("  A.Touch Chan %2d Val %2d\n", chan, Pm_MessageData1(data));
    }
  } else if (command == MIDI_BEND && bender) {
    // TODO pitch bend for a channel
      //printf("P.Bend  Chan %2d Val %2d\n", chan,
              //(Pm_MessageData1(data) + (Pm_MessageData2(data)<<7)));
  } else if (Pm_MessageStatus(data) == MIDI_SONG_POINTER) {
    // TODO unused
  } else if (Pm_MessageStatus(data) == MIDI_SONG_SELECT) {
    // TODO unused
  } else if (Pm_MessageStatus(data) == MIDI_TUNE_REQ) {
    // TODO unused
  } else if (Pm_MessageStatus(data) == MIDI_Q_FRAME && realdata) {
    //showbytes(data, 2, verbose);
    if (verbose) {
      //printf("    Time Code Quarter Frame Type %d Values %d\n",
                //(Pm_MessageData1(data) & 0x70) >> 4, Pm_MessageData1(data) & 0xf);
    }
  } else if (Pm_MessageStatus(data) == MIDI_START && realdata) {
    //showbytes(data, 1, verbose);
    if (verbose) {
      //printf("    Start\n");
    }
  } else if (Pm_MessageStatus(data) == MIDI_CONTINUE && realdata) {
    //showbytes(data, 1, verbose);
    if (verbose) {
      //printf("    Continue\n");
    }
  } else if (Pm_MessageStatus(data) == MIDI_STOP && realdata) {
    //showbytes(data, 1, verbose);
    if (verbose) {
      //printf("    Stop\n");
    }
  } else if (Pm_MessageStatus(data) == MIDI_SYS_RESET && realdata) {
    //showbytes(data, 1, verbose);
    if (verbose) {
      //printf("    System Reset\n");
    }
  } else if (Pm_MessageStatus(data) == MIDI_TIME_CLOCK) {
    if (realdata) {
      //showbytes(data, 1, verbose);
      if (verbose) {
        //printf("    Clock\n");
      }
    }
  } else if (Pm_MessageStatus(data) == MIDI_ACTIVE_SENSING) {
    if (realdata) {
      //showbytes(data, 1, verbose);
      if (verbose) {
          //printf("    Active Sensing\n");
      }
    }
  } else {
    //showbytes(data, 3, verbose);
  }
*/
}

void
receive_poll(PtTimestamp timestamp, void *userData)
{
  PmEvent event;
  int count;
  if (!active) return;

  while ((count = Pm_Read(midi_in, &event, 1))) {
    if (count == 1) {
      handle_midi_in(event.message);
    } else {
      printf(Pm_GetErrorText(count));
      fflush(stdout);
    }
  }
}

PmQueue *
midi_listener_init()
{
  Pt_Start(1, receive_poll, NULL); // start midi listener

  Pm_Initialize();

  // assume my privia keyboard is input 0
  PmError err = Pm_OpenInput(&midi_in, 0, NULL, 512, NULL, NULL);
  if (err) {
    printf(Pm_GetErrorText(err));
    Pt_Stop();
    return NULL;
  }

  midi_to_main = Pm_QueueCreate(64, sizeof(struct my_midi_st));
  return midi_to_main;
}

void
midi_cleanup()
{
  Pt_Stop();
  Pm_QueueDestroy(midi_to_main);
  Pm_Close(midi_in);
  Pm_Terminate();
}

void
midi_start()
{
  active = true;
}

void
midi_stop()
{
  active = false;
}
