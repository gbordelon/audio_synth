#include <portmidi.h>
#include <pmutil.h>
#include <porttime.h>
#include <stdbool.h>
#include <stdio.h>

#include "../lib/macros.h"
#include "midi.h"

bool active = false;     /* set when midi_in is ready for reading */
bool in_sysex = false;   /* we are reading a sysex message */
bool inited = false;     /* suppress printing during command line parsing */
bool done = false;       /* when true, exit */
bool notes = true;       /* show notes? */
bool controls = true;    /* show continuous controllers */
bool bender = true;      /* record pitch bend etc.? */
bool excldata = true;    /* record system exclusive data? */
bool verbose = true;     /* show text representation? */
bool realdata = true;    /* record real time messages? */
bool clksencnt = true;   /* clock and active sense count on */
bool chmode = true;      /* show channel mode messages */
bool pgchanges = true;   /* show program changes */
bool flush = false;	    /* flush all pending MIDI data */

uint32_t filter = 0;            /* remember state of midi filter */

uint32_t clockcount = 0;        /* count of clocks */
uint32_t actsensecount = 0;     /* cout of active sensing bytes */
uint32_t notescount = 0;        /* #notes since last request */
uint32_t notestotal = 0;        /* total #notes */

PmQueue *midi_to_main;
PmStream *midi_in;      /* midi input */

static void
handle_midi_in(PmMessage data)
{
    int command;    /* the current command */
    //int chan;   /* the midi channel of the current event */
    //int len;    /* used to get constant field width */

    /* printf("handle_midi_in data %8x; ", data); */

    command = Pm_MessageStatus(data) & MIDI_CODE_MASK;
    //chan = Pm_MessageStatus(data) & MIDI_CHN_MASK;

    if (in_sysex || Pm_MessageStatus(data) == MIDI_SYSEX) {
#define sysex_max 16
        int i;
        PmMessage data_copy = data;
        in_sysex = true;
        /* look for MIDI_EOX in first 3 bytes 
         * if realtime messages are embedded in sysex message, they will
         * be printed as if they are part of the sysex message
         */
        for (i = 0; (i < 4) && ((data_copy & 0xFF) != MIDI_EOX); i++) 
            data_copy >>= 8;
        if (i < 4) {
            in_sysex = false;
            i++; /* include the EOX byte in output */
        }
        //showbytes(data, i, verbose);
        //if (verbose) printf("System Exclusive\n");
    } else if (command == MIDI_ON_NOTE) {
        notescount++;
        // TODO
        //Pm_MessageData1(data); // note number
        //Pm_MessageData2(data); // velocity
        // convert data1 and data2 into msg
        Pm_Enqueue(midi_to_main, &data);
        if (notes) {
            //showbytes(data, 3, verbose);
            if (verbose) {
                //printf("NoteOn  Chan %2d Key %3d ", chan, Pm_MessageData1(data));
                //len = put_pitch(Pm_MessageData1(data));
                //printf(vel_format + len, Pm_MessageData2(data));
            }
        }
    } else if (command == MIDI_OFF_NOTE) {
        // TODO
        //Pm_MessageData1(data); // note number
        //Pm_MessageData2(data); // velocity
        // convert data1 and data2 into msg
        Pm_Enqueue(midi_to_main, &data);
        //showbytes(data, 3, verbose);
        if (verbose) {
            //printf("NoteOff Chan %2d Key %3d ", chan, Pm_MessageData1(data));
            //len = put_pitch(Pm_MessageData1(data));
            //printf(vel_format + len, Pm_MessageData2(data));
        }
    } else if (command == MIDI_CH_PROGRAM && pgchanges) {
        //showbytes(data, 2, verbose);
        if (verbose) {
            //printf("  ProgChg Chan %2d Prog %2d\n", chan, Pm_MessageData1(data) + 1);
        }
    } else if (command == MIDI_CTRL) {
               /* controls 121 (MIDI_RESET_CONTROLLER) to 127 are channel
                * mode messages. */
        if (Pm_MessageData1(data) < MIDI_ALL_SOUND_OFF) {
            //showbytes(data, 3, verbose);
            if (verbose) {
                //printf("CtrlChg Chan %2d Ctrl %2d Val %2d\n",
                       //chan, Pm_MessageData1(data), Pm_MessageData2(data));
            }
        } else /* channel mode */ if (chmode) {
            //showbytes(data, 3, verbose);
            if (verbose) {
                switch (Pm_MessageData1(data)) {
                  case MIDI_ALL_SOUND_OFF:
                      //printf("All Sound Off, Chan %2d\n", chan);
                    break;
                  case MIDI_RESET_CONTROLLERS:
                    //printf("Reset All Controllers, Chan %2d\n", chan);
                    break;
                  case MIDI_LOCAL:
                    //printf("LocCtrl Chan %2d %s\n",
                            //chan, Pm_MessageData2(data) ? "On" : "Off");
                    break;
                  case MIDI_ALL_OFF:
                    //printf("All Off Chan %2d\n", chan);
                    break;
                  case MIDI_OMNI_OFF:
                    //printf("OmniOff Chan %2d\n", chan);
                    break;
                  case MIDI_OMNI_ON:
                    //printf("Omni On Chan %2d\n", chan);
                    break;
                  case MIDI_MONO_ON:
                    //printf("Mono On Chan %2d\n", chan);
                    if (Pm_MessageData2(data)) {
                        //printf(" to %d received channels\n", Pm_MessageData2(data));
                    } else {
                        //printf(" to all received channels\n");
                    }
                    break;
                  case MIDI_POLY_ON:
                    //printf("Poly On Chan %2d\n", chan);
                    break;
                }
            }
        }
    } else if (command == MIDI_POLY_TOUCH && bender) {
        //showbytes(data, 3, verbose);
        if (verbose) {
            //printf("P.Touch Chan %2d Key %2d ", chan, Pm_MessageData1(data));
            //len = put_pitch(Pm_MessageData1(data));
            //printf(val_format + len, Pm_MessageData2(data));
        }
    } else if (command == MIDI_TOUCH && bender) {
        //showbytes(data, 2, verbose);
        if (verbose) {
            //printf("  A.Touch Chan %2d Val %2d\n", chan, Pm_MessageData1(data));
        }
    } else if (command == MIDI_BEND && bender) {
        //showbytes(data, 3, verbose);
        if (verbose) {
            //printf("P.Bend  Chan %2d Val %2d\n", chan,
                    //(Pm_MessageData1(data) + (Pm_MessageData2(data)<<7)));
        }
    } else if (Pm_MessageStatus(data) == MIDI_SONG_POINTER) {
        //showbytes(data, 3, verbose);
        if (verbose) {
            //printf("    Song Position %d\n",
                    //(Pm_MessageData1(data) + (Pm_MessageData2(data)<<7)));
        }
    } else if (Pm_MessageStatus(data) == MIDI_SONG_SELECT) {
        //showbytes(data, 2, verbose);
        if (verbose) {
            //printf("    Song Select %d\n", Pm_MessageData1(data));
        }
    } else if (Pm_MessageStatus(data) == MIDI_TUNE_REQ) {
        //showbytes(data, 1, verbose);
        if (verbose) {
            //printf("    Tune Request\n");
        }
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
        if (clksencnt) clockcount++;
        else if (realdata) {
            //showbytes(data, 1, verbose);
            if (verbose) {
                //printf("    Clock\n");
            }
        }
    } else if (Pm_MessageStatus(data) == MIDI_ACTIVE_SENSING) {
        if (clksencnt) actsensecount++;
        else if (realdata) {
            //showbytes(data, 1, verbose);
            if (verbose) {
                //printf("    Active Sensing\n");
            }
        }
    } else {
      //showbytes(data, 3, verbose);
    }
    //fflush(stdout);
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

  midi_to_main = Pm_QueueCreate(32, sizeof(int32_t));
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

