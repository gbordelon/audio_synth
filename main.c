#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <AudioUnit/AudioUnit.h>

#include "portmidi.h"
#include "pmutil.h"
#include "porttime.h"

#include "src/lib/macros.h"

#include "src/mac_audio/audio_unit.h"

#include "src/dsp/audio_filter.h"
#include "src/dsp/bitcrusher.h"
#include "src/dsp/dsp.h"
#include "src/env/envelope.h"
#include "src/midi/midi.h"
#include "src/midi/tunable.h"
#include "src/pcm/mixer.h"
#include "src/pcm/pcm.h"
#include "src/ugen/sin.h"
#include "src/ugen/ugen.h"
#include "src/voice/voice.h"
#include "src/voice/dx7.h"

#define MIDI_CODE_MASK  0xf0
#define MIDI_CHN_MASK   0x0f
#define MIDI_OFF_NOTE   0x80
#define MIDI_ON_NOTE    0x90
#define MIDI_POLY_TOUCH 0xa0
#define MIDI_CTRL       0xb0
#define MIDI_CH_PROGRAM 0xc0
#define MIDI_TOUCH      0xd0
#define MIDI_BEND       0xe0

#define MIDI_SYSEX      0xf0
#define MIDI_Q_FRAME	0xf1
#define MIDI_SONG_POINTER 0xf2
#define MIDI_SONG_SELECT 0xf3
#define MIDI_TUNE_REQ	0xf6
#define MIDI_EOX        0xf7
#define MIDI_TIME_CLOCK 0xf8
#define MIDI_START      0xfa
#define MIDI_CONTINUE	0xfb
#define MIDI_STOP       0xfc
#define MIDI_ACTIVE_SENSING 0xfe
#define MIDI_SYS_RESET  0xff

#define MIDI_ALL_SOUND_OFF 0x78
#define MIDI_RESET_CONTROLLERS 0x79
#define MIDI_LOCAL	0x7a
#define MIDI_ALL_OFF	0x7b
#define MIDI_OMNI_OFF	0x7c
#define MIDI_OMNI_ON	0x7d
#define MIDI_MONO_ON	0x7e
#define MIDI_POLY_ON	0x7f

PmStream *midi_in;      /* midi input */
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

Mixer gmix;
Voice gsynth;
Voice gmic;

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

#define add_filter(vc, fx, params, filter_type, Fc, Q, dB)\
  (params).alg=(filter_type);\
  (params).fc=(Fc);\
  (params).q=(Q);\
  (params).boost_cut_db=(dB);\
  fx##_l = dsp_init_audio_filter(params);\
  fx##_r = dsp_init_audio_filter(params);\
  (fx##_r)->fn_type = DSP_MONO_R;\
  (vc)->fx_chain = dsp_add_to_chain(vc->fx_chain, fx##_l);\
  (vc)->fx_chain = dsp_add_to_chain(vc->fx_chain, fx##_r)
   
int
main()
{
  PmError err;
  uint8_t active_voices[128];
  printf("generating wave tables... ");
  fflush(stdout);

  clock_t t = clock();
  ugen_generate_tables();
  t = clock() - t;
  double tt = t / (double)CLOCKS_PER_SEC;
  printf("took %f seconds.\n", tt);

  gmix = mixer_init(2, 1.0);
  printf("mixer initialized.\n");

/* gsynth */
  Channel chans = gmix->busses[0].channels;
  //gsynth = voice_init_default(chans, NUM_CHANNELS);
  mono_voice_params mv_params;
  dx7_e_piano_1(&mv_params);
  gsynth = voice_init(chans, NUM_CHANNELS, VOICE_DX7, mv_params);

  DSP_callback cb = dsp_init_envelope_follower_default();
  //gsynth->fx_chain = dsp_add_to_chain(gsynth->fx_chain, cb);
  cb = dsp_init_envelope_follower_default();
  cb->fn_type = DSP_MONO_R;
  //gsynth->fx_chain = dsp_add_to_chain(gsynth->fx_chain, cb);

  DSP_callback dsp_fx_l, dsp_fx_r;
  // precede env follower with a bitcrusher on each channel
  dsp_fx_l = dsp_init_bitcrusher();
  dsp_set_bitcrusher_param(&dsp_fx_l->state, 4.5);

  dsp_fx_r = dsp_init_bitcrusher();
  dsp_fx_r->fn_type = DSP_MONO_R;
  dsp_set_bitcrusher_param(&dsp_fx_r->state, 4.5);

  //gsynth->fx_chain = dsp_add_to_chain(gsynth->fx_chain, dsp_fx_l);
  //gsynth->fx_chain = dsp_add_to_chain(gsynth->fx_chain, dsp_fx_r);

  Ugen ug;
/*
  ug = ugen_init_tri(0.05);
  ugen_set_scale(ug, 0.3, 0.7);
  dsp_set_control_ugen(gsynth->fx_chain, ug);
*/
/* end gsynth */

/* gmic */
  chans = gmix->busses[1].channels;
  gmic = voice_init(chans, NUM_CHANNELS, VOICE_MIC_IN, mv_params);

  // set slow triangle stereo pan on gmic
  gmic->fx_chain = dsp_init_stereo_pan();
//  ug = ugen_init_tri(0.08);
//  ugen_set_scale(ug, 0.3, 0.7);
//  dsp_set_control_ugen(gmic->fx_chain, ug);

  audio_filter_params params;
  // telephone style filter uses a LPF at 4k and a HPF at 400
  add_filter(gmic, dsp_fx, params, AF_HPF2, 400.0, 5.707, 0.0);
  add_filter(gmic, dsp_fx, params, AF_LPF2, 4000.0, 5.707, 0.0);

/* end gmic */

  printf("instrument initialized.\n");

  AudioComponentInstance audio_unit_io = audio_unit_io_init();
  printf("AudioUnit io initialized.\n");

  memset(active_voices, 64, 128);

  Pt_Start(1, receive_poll, NULL); // start midi listener

  Pm_Initialize();

  // assume my privia keyboard is input 0
  err = Pm_OpenInput(&midi_in, 0, NULL, 512, NULL, NULL);
  if (err) {
      printf(Pm_GetErrorText(err));
      Pt_Stop();
      voice_cleanup(gsynth);
      mixer_cleanup(gmix);
      exit(1);
  }

  midi_to_main = Pm_QueueCreate(32, sizeof(int32_t));
  // check midi_to_main != NULL

  active = true;
  printf("MIDI in initialized.\n");

  audio_unit_go(audio_unit_io);
  printf("Synth started.\n");
  fflush(stdout);

  gmix->gain = 0.7;
  //voice_note_on(gmic, 30, 127);

  int32_t msg;
  int command;    /* the current command */

  uint8_t note_ind;
  
  int spin;
  int num_active_notes = 0;
  for (;;) {
    // read midi messages
    spin = Pm_Dequeue(midi_to_main, &msg);
    if (spin) {
      command = Pm_MessageStatus(msg) & MIDI_CODE_MASK;
      if (command == MIDI_ON_NOTE) {
        if (active_voices[Pm_MessageData1(msg)] == 64) {
          note_ind = voice_note_on(gsynth, Pm_MessageData1(msg), Pm_MessageData2(msg));
          active_voices[Pm_MessageData1(msg)] = note_ind;
          num_active_notes++;
        } else {
        }
      } else if (command == MIDI_OFF_NOTE) {
        if (active_voices[Pm_MessageData1(msg)] < 64) {
          voice_note_off(gsynth, active_voices[Pm_MessageData1(msg)]);
          active_voices[Pm_MessageData1(msg)] = 64;
          num_active_notes--;
        }
      } else if (command == MIDI_CTRL) {
        if (Pm_MessageData1(msg) < MIDI_ALL_SOUND_OFF) {
           //tunable_set((FTYPE)Pm_MessageData2(msg) / 127.0, Pm_MessageData1(msg));
        } else {
        }
      }
    }
  }
  active = false;

  Pt_Stop();
  Pm_QueueDestroy(midi_to_main);
  Pm_Close(midi_in);
  Pm_Terminate();

  voice_cleanup(gmic);
  voice_cleanup(gsynth);
  mixer_cleanup(gmix);
  return 0;
}

/*
#include <math.h>
#include <stdio.h>

  FTYPE max = 0;
        if (*write_buf_r * *write_buf_r > max) {
          max = *write_buf_r * *write_buf_r;
        }
  printf("max sample mixed for this chunk: %f\n", sqrt(max));
*/
