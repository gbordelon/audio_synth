#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include <AudioUnit/AudioUnit.h>

#include "../lib/macros.h"
#include "../pcm/mixer.h"
#include "../voice/voice.h"

#include "audio_unit.h"

#define MMAP_SIZE (NUM_CHANNELS*CHUNK_SIZE)
#define kOutputBus 0
#define kInputBus 1

extern Mixer gmix;
extern Voice gvoice;
static size_t _index = 0;
FTYPE *_buffer;

void
checkStatus(OSStatus status)
{
}

static OSStatus
playbackCallback(void *inRefCon, 
                 AudioUnitRenderActionFlags *ioActionFlags, 
                 const AudioTimeStamp *inTimeStamp, 
                 UInt32 inBusNumber, 
                 UInt32 inNumberFrames, 
                 AudioBufferList *ioData);

AudioComponentInstance
audio_unit_init()
{
  OSStatus status;
  AudioComponentInstance audioUnit;

  // Describe audio component
  AudioComponentDescription desc;
  desc.componentType = kAudioUnitType_Output;
  desc.componentSubType = kAudioUnitSubType_DefaultOutput;//kAudioUnitSubType_RemoteIO;
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;

  // Get component
  AudioComponent inputComponent = AudioComponentFindNext(NULL, &desc);

  // Get audio units
  status = AudioComponentInstanceNew(inputComponent, &audioUnit);
  checkStatus(status);

  UInt32 flag = 1;
  // Enable IO for playback
  status = AudioUnitSetProperty(audioUnit, 
                  kAudioOutputUnitProperty_EnableIO, 
                  kAudioUnitScope_Output, 
                  kOutputBus,
                  &flag, 
                  sizeof(flag));
  checkStatus(status);

  // Describe format

  AudioStreamBasicDescription audioFormat;
  audioFormat.mSampleRate = DEFAULT_SAMPLE_RATE;
  audioFormat.mFormatID = kAudioFormatLinearPCM;
  audioFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
  audioFormat.mFramesPerPacket = 1;
  audioFormat.mChannelsPerFrame = NUM_CHANNELS;
  audioFormat.mBitsPerChannel = CHAR_BIT * sizeof(FTYPE);
  audioFormat.mBytesPerPacket = sizeof(FTYPE) * NUM_CHANNELS;
  audioFormat.mBytesPerFrame = sizeof(FTYPE) * NUM_CHANNELS;

  // Apply format

  status = AudioUnitSetProperty(audioUnit, 
                  kAudioUnitProperty_StreamFormat, 
                  kAudioUnitScope_Input, 
                  kOutputBus, 
                  &audioFormat, 
                  sizeof(audioFormat));
  checkStatus(status);

  // Set output callback
  AURenderCallbackStruct callbackStruct;
  callbackStruct.inputProc = playbackCallback;
  callbackStruct.inputProcRefCon = NULL;
  status = AudioUnitSetProperty(audioUnit, 
                  kAudioUnitProperty_SetRenderCallback, 
                  kAudioUnitScope_Global, 
                  kOutputBus,
                  &callbackStruct, 
                  sizeof(callbackStruct));

  // Initialize
  status = AudioUnitInitialize(audioUnit);

  _buffer = calloc(MMAP_SIZE, sizeof(FTYPE));
  return audioUnit;
}

static void
pull_samples()
{
  voice_play_chunk(gvoice);
  mixer_update(gmix);
  memcpy(_buffer, gmix->write_buf, MMAP_SIZE * sizeof(FTYPE));
  memset(gmix->write_buf, 0, MMAP_SIZE * sizeof(FTYPE));
}

static OSStatus
playbackCallback(void *inRefCon, 
                 AudioUnitRenderActionFlags *ioActionFlags, 
                 const AudioTimeStamp *inTimeStamp, 
                 UInt32 inBusNumber, 
                 UInt32 inNumberFrames, 
                 AudioBufferList *ioData) 
{
  if (_index >= MMAP_SIZE) {
    pull_samples();
    _index = 0;
  }

  for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i) {
    int samplesLeft = MMAP_SIZE - _index; // mixer buffer number of frames
    int numSamples = ioData->mBuffers[i].mDataByteSize / sizeof(FTYPE);

    if (samplesLeft > 0) {
      if(samplesLeft < numSamples) {
        //printf("Generating another chunk to get %d samples\n", numSamples - samplesLeft);
        memcpy(ioData->mBuffers[i].mData, _buffer + _index, samplesLeft * sizeof(FTYPE));
        _index += samplesLeft;

        pull_samples();
        memcpy(((FTYPE*)ioData->mBuffers[i].mData) + samplesLeft, _buffer, (numSamples - samplesLeft) * sizeof(FTYPE));
        _index = numSamples - samplesLeft;
      } else {
        //printf("fully copied %d samples\n", numSamples);
        memcpy(ioData->mBuffers[i].mData, _buffer + _index, numSamples * sizeof(FTYPE));
        _index += numSamples;
      }
    } else {
      printf("fully setting buffer 0\n");
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
  checkStatus(status);
}
