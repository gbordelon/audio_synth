#include <stddef.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#include "../lib/macros.h"
#include "../fx/buffer.h"
#include "../fx/fx.h"
#include "../pcm/mixer.h"
#include "../voice/voice.h"

#include "audio_unit.h"

#define ENABLE_INPUT 1
#define USE_MIC 0

#define MMAP_SIZE (NUM_CHANNELS*CHUNK_SIZE)
#define kOutputBus 0
#define kInputBus 1

extern Voice gsynth[2];
extern char const * icky_global_program_name;

AudioComponentInstance _audioUnit;

static size_t _output_index = 0;
FTYPE *_output_buffer;
AudioBufferList *_input_buffer;

size_t _mic_output_write_index = 0;
size_t _mic_output_read_index = 0;
FTYPE *_mic_output_buffer_L;
FTYPE *_mic_output_buffer_R;
FTYPE *_mic_output_buffer_stereo;

static const FTYPE gain = 1.0; //0dB //2.0;//pow(10.0, 6.0 / 20.0); // dB

#define fx_unit_input_buffer_idx 1
#define fx_unit_output_buffer_idx 2

void
checkStatus(OSStatus status)
{
  //printf("%d\n", status);
  //fflush(stdout);
}

#if ENABLE_INPUT
static OSStatus
inputCallback(void *inRefCon, 
                 AudioUnitRenderActionFlags *ioActionFlags, 
                 const AudioTimeStamp *inTimeStamp, 
                 UInt32 inBusNumber, 
                 UInt32 inNumberFrames, 
                 AudioBufferList *ioData);
#endif

static OSStatus
playbackCallback(void *inRefCon, 
                 AudioUnitRenderActionFlags *ioActionFlags, 
                 const AudioTimeStamp *inTimeStamp, 
                 UInt32 inBusNumber, 
                 UInt32 inNumberFrames, 
                 AudioBufferList *ioData);

AudioComponentInstance
audio_unit_io_init()
{
  OSStatus status;

  // Describe audio component
  AudioComponentDescription desc;
  desc.componentType = kAudioUnitType_Output;
#if ENABLE_INPUT
  #if USE_MIC
  desc.componentSubType = kAudioUnitSubType_VoiceProcessingIO; // has special behavior to avoid feedback with lappy speakers.
  #else
  desc.componentSubType = kAudioUnitSubType_HALOutput;
  #endif
#else
  desc.componentSubType = kAudioUnitSubType_DefaultOutput;
#endif
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;

  // Get component
  AudioComponent inputComponent = AudioComponentFindNext(NULL, &desc);

  // Get audio units
  status = AudioComponentInstanceNew(inputComponent, &_audioUnit);
  //printf("making a new voice_processingIO audioUnit ");
  checkStatus(status);

  // Enable IO for recording
#if ENABLE_INPUT
  UInt32 flag = 1;
  status = AudioUnitSetProperty(_audioUnit,
                                kAudioOutputUnitProperty_EnableIO,
                                kAudioUnitScope_Input,
                                kInputBus,
                                &flag,
                                sizeof(flag));
  //printf("enable input ");
  checkStatus(status);

  // this is set by default so i don't need to place it outside the if ENABLE_INPUT
  // Enable IO for playback
  flag = 1;
  status = AudioUnitSetProperty(_audioUnit,
                                kAudioOutputUnitProperty_EnableIO,
                                kAudioUnitScope_Output,
                                kOutputBus,
                                &flag,
                                sizeof(flag));
  //printf("enable output ");
  checkStatus(status);
#endif

  // Apply format
  AudioStreamBasicDescription audioFormat;
  audioFormat.mSampleRate = DEFAULT_SAMPLE_RATE;
  audioFormat.mFormatID = kAudioFormatLinearPCM;
  audioFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
  audioFormat.mFramesPerPacket = 1;
#if ENABLE_INPUT
  audioFormat.mBitsPerChannel = CHAR_BIT * sizeof(Float32);
  #if USE_MIC
  audioFormat.mChannelsPerFrame = 1;
  audioFormat.mBytesPerPacket = sizeof(Float32);
  audioFormat.mBytesPerFrame = sizeof(Float32);
  #else
  audioFormat.mChannelsPerFrame = NUM_CHANNELS;
  audioFormat.mBytesPerPacket = sizeof(Float32) * NUM_CHANNELS;
  audioFormat.mBytesPerFrame = sizeof(Float32) * NUM_CHANNELS;
  #endif
  status = AudioUnitSetProperty(_audioUnit,
                                kAudioUnitProperty_StreamFormat,
                                kAudioUnitScope_Output,
                                kInputBus,
                                &audioFormat,
                                sizeof(audioFormat));
  //printf("set input stream properties ");
  checkStatus(status);
#endif
  audioFormat.mChannelsPerFrame = NUM_CHANNELS;
  audioFormat.mBitsPerChannel = CHAR_BIT * sizeof(FTYPE);
  audioFormat.mBytesPerPacket = sizeof(FTYPE) * NUM_CHANNELS;
  audioFormat.mBytesPerFrame = sizeof(FTYPE) * NUM_CHANNELS;

  status = AudioUnitSetProperty(_audioUnit,
                                kAudioUnitProperty_StreamFormat,
                                kAudioUnitScope_Input,
                                kOutputBus,
                                &audioFormat,
                                sizeof(audioFormat));
  //printf("set output stream properties ");
  checkStatus(status);

  AURenderCallbackStruct callbackStruct;
#if ENABLE_INPUT
  // Set input callback
  callbackStruct.inputProc = inputCallback;
  callbackStruct.inputProcRefCon = NULL;
  status = AudioUnitSetProperty(_audioUnit,
                                kAudioOutputUnitProperty_SetInputCallback,
                                kAudioUnitScope_Global,
                                kInputBus,
                                &callbackStruct,
                                sizeof(callbackStruct));
  //printf("set input callback ");
  checkStatus(status);
#endif
  // Set output callback
  callbackStruct.inputProc = playbackCallback;
  callbackStruct.inputProcRefCon = NULL;
  status = AudioUnitSetProperty(_audioUnit,
                                kAudioUnitProperty_SetRenderCallback,
                                kAudioUnitScope_Global,
                                kOutputBus,
                                &callbackStruct, 
                                sizeof(callbackStruct));
  //printf("set output callback ");
  checkStatus(status);

#if ENABLE_INPUT
  // Disable buffer allocation for the recorder (optional - do this if we want to pass in our own)
  flag = 0;
  status = AudioUnitSetProperty(_audioUnit,
                                kAudioUnitProperty_ShouldAllocateBuffer,
                                kAudioUnitScope_Output, 
                                kInputBus,
                                &flag, 
                                sizeof(flag));

  //printf("disable input buffer allocation ");
  checkStatus(status);
#endif

  // Initialise
  status = AudioUnitInitialize(_audioUnit);

  //printf("audioUnit initialization ");
  checkStatus(status);

#if ENABLE_INPUT
  // allocate bufferlist for reading mic data
  UInt32 bufferSizeBytes = 512 * sizeof(Float32) * NUM_CHANNELS * 2; // 512 frames but double it
  UInt32 propertySize = offsetof(AudioBufferList, mBuffers[0]) + (sizeof(AudioBuffer) * NUM_CHANNELS);

  _input_buffer = (AudioBufferList *) calloc(1, propertySize);
  _input_buffer->mNumberBuffers = 1; // channels per frame

  UInt32 i;
  for(i = 0; i < _input_buffer->mNumberBuffers; ++i)
  {
  #if USE_MIC
    _input_buffer->mBuffers[i].mNumberChannels = 1;
  #else
    _input_buffer->mBuffers[i].mNumberChannels = NUM_CHANNELS;
  #endif
    _input_buffer->mBuffers[i].mDataByteSize = bufferSizeBytes;
    _input_buffer->mBuffers[i].mData = calloc(1, bufferSizeBytes);
  }

  //_mic_output_buffer_L = calloc(8 * MMAP_SIZE, sizeof(FTYPE));
  //_mic_output_buffer_R = calloc(8 * MMAP_SIZE, sizeof(FTYPE));
  _mic_output_buffer_stereo = calloc(MMAP_SIZE, sizeof(FTYPE));
#endif
  _output_buffer = calloc(MMAP_SIZE, sizeof(FTYPE));

  return _audioUnit;
}

#if ENABLE_INPUT
static OSStatus
inputCallback(void *inRefCon, 
                 AudioUnitRenderActionFlags *ioActionFlags, 
                 const AudioTimeStamp *inTimeStamp, 
                 UInt32 inBusNumber, 
                 UInt32 inNumberFrames, 
                 AudioBufferList *ioData)
{
  OSStatus err = noErr;

  err = AudioUnitRender(_audioUnit, // input audio unit
                  ioActionFlags,
                  inTimeStamp,
                  inBusNumber,     //will be '1' for input data
                  inNumberFrames, //# of frames requested
                  _input_buffer);

  checkStatus(err);

  if (inNumberFrames > 1024) {
    //printf("inputCallback wants to render more than 1024 frames: %d\n", inNumberFrames);
  } else if (inNumberFrames < 1024) {
    //printf("inputCallback wants to render less than 1024 frames: %d\n", inNumberFrames);
  } else {
    //printf("inputCallback wants to render 1024 frames: %d\n", inNumberFrames);
  }

  if (_mic_output_write_index >= MMAP_SIZE) {
    _mic_output_write_index = 0;
  }

  // convert to FTYPE and copy into _mic_output_buffer + _mic_output_index
  UInt32 i;
  int numSamples = inNumberFrames; // number of frames to copy at 1 (or 2) channel per frame
  for (i = 0; i < _input_buffer->mNumberBuffers; ++i) { // should only be 1 buffer
    int samplesLeft = MMAP_SIZE - _mic_output_write_index; // remaining amount of sample space
    int samplesAfterWrap;

    if (samplesLeft > 0) {
      Float32 *samp = (Float32 *)_input_buffer->mBuffers[i].mData; // data is stereo interleaved frames
#if USE_MIC
      //Float32 max = 0;
      //if (*samp * *samp > max) max = *samp * *samp;

      if(samplesLeft < numSamples) { // wrap around to the start of the buffer
        for (samplesAfterWrap = samplesLeft; samplesAfterWrap > 0; samplesAfterWrap--, samp++, _mic_output_write_index++) {
          _mic_output_buffer[_mic_output_write_index] = *samp;
          //if (*samp * *samp > max) max = *samp * *samp;
        }
        for (samplesAfterWrap = numSamples - samplesLeft, _mic_output_write_index = 0; samplesAfterWrap > 0; samplesAfterWrap--, samp++, _mic_output_write_index++) {
          _mic_output_buffer[_mic_output_write_index] = *samp;
          //if (*samp * *samp > max) max = *samp * *samp;
        }
      } else {
        for (samplesAfterWrap = numSamples; samplesAfterWrap > 0; samplesAfterWrap--, samp++, _mic_output_write_index++) {
          _mic_output_buffer[_mic_output_write_index] = *samp;
          //if (*samp * *samp > max) max = *samp * *samp;
        }
      }
      //printf("max sample input for this chunk: %f\n", sqrt(max));
#else

      if (fabs(*samp) > 1.0 || fabs(*(samp + 1)) > 1.0) {
        printf("peak: %f %f\n", *samp, *(samp+1));
      }

      if(samplesLeft < numSamples) { // wrap around to the start of the buffer
        for (samplesAfterWrap = samplesLeft; samplesAfterWrap > 0; samplesAfterWrap--, samp+=2, _mic_output_write_index+=2) {
          //_mic_output_buffer_L[_mic_output_write_index] = *samp;
          //_mic_output_buffer_R[_mic_output_write_index] = *(samp+1);
          _mic_output_buffer_stereo[_mic_output_write_index] = *samp * gain;
          _mic_output_buffer_stereo[_mic_output_write_index + 1] = *(samp+1) * gain;
        }
        for (samplesAfterWrap = numSamples - samplesLeft, _mic_output_write_index = 0; samplesAfterWrap > 0; samplesAfterWrap--, samp+=2, _mic_output_write_index+=2) {
          //_mic_output_buffer_L[_mic_output_write_index] = *samp;
          //_mic_output_buffer_R[_mic_output_write_index] = *(samp+1);
          _mic_output_buffer_stereo[_mic_output_write_index] = *samp * gain;
          _mic_output_buffer_stereo[_mic_output_write_index + 1] = *(samp+1) * gain;
        }
      } else {
        for (samplesAfterWrap = numSamples; samplesAfterWrap > 0; samplesAfterWrap--, samp+=2, _mic_output_write_index+=2) {
          //_mic_output_buffer_L[_mic_output_write_index] = *samp;
          //_mic_output_buffer_R[_mic_output_write_index] = *(samp+1);
          _mic_output_buffer_stereo[_mic_output_write_index] = *samp * gain;
          _mic_output_buffer_stereo[_mic_output_write_index + 1] = *(samp+1) * gain;
        }
      }
#endif
    } else {
    }
  }

  return noErr;
}
#endif

static void
pull_samples()
{
  // generate a chunk of samples
  if (strcmp(icky_global_program_name, "./synth") == 0) {
    voice_play_chunk(gsynth[0]);
    voice_play_chunk(gsynth[1]);
  }

  fx_unit_buffer_write_chunk(fx_unit_input_buffer_idx, _mic_output_buffer_stereo, _mic_output_write_index);

  int i;
  FTYPE rv[2] = {0};
  for (i = 0; i < CHUNK_SIZE; i++) {
//    fx_unit_process_frame(fx_unit_output_buffer_idx);
//    fx_unit_reset_output_buffers();
    fx_unit_entry_point(rv, fx_unit_output_buffer_idx);
    if (fabs(rv[0]) > 1.0 || fabs(rv[1]) > 1.0) {
      printf("%f %f\n", rv[0], rv[1]);
    }
  }

  // copy mixer buffer data to mac audio buffer
  fx_unit_buffer_read_chunk(fx_unit_output_buffer_idx, _output_buffer);

}

// called about every 10 millis
static OSStatus
playbackCallback(void *inRefCon, 
                 AudioUnitRenderActionFlags *ioActionFlags, 
                 const AudioTimeStamp *inTimeStamp, 
                 UInt32 inBusNumber, 
                 UInt32 inNumberFrames, 
                 AudioBufferList *ioData) 
{
  if (_output_index >= MMAP_SIZE) {
    pull_samples();
    _output_index = 0;
  }

  for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i) {
    int samplesLeft = MMAP_SIZE - _output_index; // mixer buffer number of frames
    int numSamples = ioData->mBuffers[i].mDataByteSize / sizeof(FTYPE);
    if (samplesLeft > 0) {
      if(samplesLeft < numSamples) {
        memcpy(ioData->mBuffers[i].mData, _output_buffer + _output_index, samplesLeft * sizeof(FTYPE));
        _output_index += samplesLeft;

        pull_samples();
        memcpy(((FTYPE*)ioData->mBuffers[i].mData) + samplesLeft, _output_buffer, (numSamples - samplesLeft) * sizeof(FTYPE));
        _output_index = numSamples - samplesLeft;
      } else {
        memcpy(ioData->mBuffers[i].mData, _output_buffer + _output_index, numSamples * sizeof(FTYPE));
        _output_index += numSamples;
      }
    } else {
      //printf("fully setting buffer 0\n");
      memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
    }
  }

  return noErr;
}

void
audio_unit_go(AudioComponentInstance audioUnit)
{
  // Start playing
  OSStatus status;
  status = AudioOutputUnitStart(audioUnit);
  //printf("start audioUnit ");
  checkStatus(status);
}
