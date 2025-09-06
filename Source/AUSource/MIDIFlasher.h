// 
// MIDIFlasher.h 
//
// Copyright (c) 2020-2025 Larry M. Taylor
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software. Permission is granted to anyone to
// use this software for any purpose, including commercial applications, and to
// to alter it and redistribute it freely, subject to 
// the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source
//    distribution.
//

#include "AUBase.h"
#include "AUInstrumentBase.h"
#include <CoreMIDI/CoreMIDI.h>
#include <vector>
#include <os/log.h>

#include "LTMidi.h"

#define LT_NO_COCOA
#include "LTLog.h"

#define kMIDIFlasherVersion 65540

// Custom properties IDs must be 64000 or greater
// See AudioUnit/AudioUnitProperties.h for a list of
// Apple-defined standard properties
#define kAudioUnitCustomPropertyUICB 64056

typedef struct LTMIDIMessageInfoStruct
{
    UInt8 status;
    UInt8 channel;
    UInt8 data1;
    UInt8 data2;
    UInt32 startFrame;
} LTMIDIMessageInfoStruct;


class MIDIOutputCallbackHelper 
{
    enum  { kSizeofMIDIBuffer = 512 };

public:
    MIDIOutputCallbackHelper() 
    {
        mMIDIMessageList.reserve(kSizeofMIDIBuffer / 8);
        mMIDICallbackStruct.midiOutputCallback = NULL;
        mMIDIBuffer = new Byte[kSizeofMIDIBuffer];
    }

    ~MIDIOutputCallbackHelper() 
    {
        delete [] mMIDIBuffer;
    }

    void SetCallbackInfo(AUMIDIOutputCallback &callback, void *userData)
    {
        mMIDICallbackStruct.midiOutputCallback = callback;
        mMIDICallbackStruct.userData = userData;
    }

    void AddMIDIEvent(UInt8 status, UInt8 channel, UInt8 data1,
                      UInt8 data2, UInt32 inStartFrame);
                            
    void FireAtTimeStamp(const AudioTimeStamp &inTimeStamp, os_log_t mLog);

private:
    MIDIPacketList *PacketList() 
    {
        return (MIDIPacketList *)mMIDIBuffer; 
    }

    Byte *mMIDIBuffer;

    AUMIDIOutputCallbackStruct mMIDICallbackStruct;

    typedef std::vector<LTMIDIMessageInfoStruct> MIDIMessageList;
    MIDIMessageList mMIDIMessageList;
};

class MIDIFlasher : public AUMonotimbralInstrumentBase
{
public:
    MIDIFlasher(AudioUnit inComponentInstance);
   
    virtual ~MIDIFlasher();

    virtual OSStatus Initialize();
    virtual void Cleanup();
    virtual OSStatus Version() { return kMIDIFlasherVersion; }

    virtual AUElement *CreateElement(AudioUnitScope scope,
                                     AudioUnitElement element);
    virtual OSStatus GetParameterInfo(AudioUnitScope inScope,
        AudioUnitParameterID inParameterID,
        AudioUnitParameterInfo &outParameterInfo);
    virtual OSStatus GetPropertyInfo(AudioUnitPropertyID inID,
                                     AudioUnitScope inScope,
                                     AudioUnitElement inElement,
                                     UInt32 &outDataSize,
                                     Boolean &outWritable);
    virtual OSStatus GetProperty(AudioUnitPropertyID inID,
                                 AudioUnitScope inScope,
                                 AudioUnitElement inElement,
                                 void *outData);
    virtual OSStatus SetProperty(AudioUnitPropertyID inID,
                                 AudioUnitScope inScope,
                                 AudioUnitElement inElement,
                                 const void *inData,
                                 UInt32 inDataSize);
                                                        
    OSStatus HandleMidiEvent(UInt8 status, UInt8 channel, UInt8 data1, 
                             UInt8 data2, UInt32 inStartFrame);

    OSStatus Render(AudioUnitRenderActionFlags &ioActionFlags,
                    const AudioTimeStamp &inTimeStamp, UInt32 inNumberFrames);

    MidiControls *GetControls(MusicDeviceGroupID inChannel)
    {
        SynthGroupElement *group = GetElForGroupID(inChannel);
        return (MidiControls *)group->GetMIDIControlHandler();
    }

private:
    MIDIOutputCallbackHelper mCallbackHelper;
    MIDIOutputCallbackHelper mUICB;
    os_log_t mLog;
};
