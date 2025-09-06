// 
// MIDIFlasher.cpp 
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

#include "MIDIFlasher.h"


void MIDIOutputCallbackHelper::AddMIDIEvent(UInt8 status, UInt8 channel,
                                            UInt8 data1, UInt8 data2, 
                                            UInt32 inStartFrame) 
{
    LTMIDIMessageInfoStruct info =
        { status, channel, data1, data2, inStartFrame };
    mMIDIMessageList.push_back(info);
}

void MIDIOutputCallbackHelper::
    FireAtTimeStamp(const AudioTimeStamp &inTimeStamp, os_log_t log) 
{
    if (!mMIDIMessageList.empty())
    {
        if (mMIDICallbackStruct.midiOutputCallback != NULL)
        {
            // Synthesize the packet list and call the MIDIOutputCallback
            // iterate through the vector and get each item
            MIDIPacketList *pktlist = PacketList();
            MIDIPacket *pkt = MIDIPacketListInit(pktlist);
            
            for (MIDIMessageList::iterator iter = mMIDIMessageList.begin();
                 iter != mMIDIMessageList.end(); iter++) 
            {
                const LTMIDIMessageInfoStruct &item = *iter;

                // Echo the incoming message
                MIDITimeStamp time =  item.startFrame +
                                      inTimeStamp.mSampleTime;
                Byte midiStatusByte = item.status | item.channel;
                Byte data[3] = { midiStatusByte, item.data1, item.data2 };
                UInt32 midiDataCount = ((item.status == MIDI_PROGRAM_CHANGE ||
                                         item.status == MIDI_SET_PRESSURE) ?
                                         2 : 3);
                pkt = MIDIPacketListAdd(pktlist, kSizeofMIDIBuffer, pkt,
                                        time, midiDataCount, data);

                if (!pkt)
                {
                    // Send what we have and then clear the buffer
                    // and then go through this again
                    // issue the callback with what we got
                    OSStatus result = (*mMIDICallbackStruct.midiOutputCallback)
                        (mMIDICallbackStruct.userData, &inTimeStamp,
                         0, pktlist);
                    
                    if (result != noErr)
                    {
                        char str[LT_AU_MESSAGE_LENGTH] = { 0 };
                        statusToString(result, str);
                        os_log_with_type(log, OS_LOG_TYPE_ERROR,
                            "MIDIFlasher FireAtTimestamp "
                            "error #1 = %i (%{public}s)", result, str);
                    }
                    
                    // Clear stuff we've already processed, and fire again
                    mMIDIMessageList.erase(mMIDIMessageList.begin(), iter);
                    FireAtTimeStamp(inTimeStamp, log);
                    return;
                }
            }
            
            // Fire callback
            OSStatus result = (*mMIDICallbackStruct.midiOutputCallback)
                (mMIDICallbackStruct.userData, &inTimeStamp, 0, pktlist);

            if (result != noErr)
            {
                char str[LT_AU_MESSAGE_LENGTH] = { 0 };
                statusToString(result, str);
                os_log_with_type(log, OS_LOG_TYPE_ERROR,
                    "MIDIFlasher FireAtTimestamp error #2 = %i (%{public}s)",
                    result, str);
            }
        }
        
        mMIDIMessageList.clear();
    }
}

AUDIOCOMPONENT_ENTRY(AUMusicDeviceFactory, MIDIFlasher)

// We have no inputs, one output
MIDIFlasher::MIDIFlasher(AudioUnit inComponentInstance)
    : AUMonotimbralInstrumentBase(inComponentInstance, 0, 1)
{
    mLog = os_log_create("com.larrymtaylor.au.MIDIFlasher", "AU");

    os_log_with_type(mLog, OS_LOG_TYPE_INFO, "MIDIFlasher::MIDIFlasher");

    CreateElements();
}

MIDIFlasher::~MIDIFlasher()
{
    os_log_with_type(mLog, OS_LOG_TYPE_INFO, "MIDIFlasher::~MIDIFlasher");
}

void MIDIFlasher::Cleanup()
{
}

OSStatus MIDIFlasher::Initialize()
{ 
    os_log_with_type(mLog, OS_LOG_TYPE_INFO, "MIDIFlasher::Initialize");

    AUMonotimbralInstrumentBase::Initialize();

    return noErr;
}

AUElement *MIDIFlasher::CreateElement(AudioUnitScope scope,
                                      AudioUnitElement element)
{
    switch (scope)
    {
        case kAudioUnitScope_Group:
            os_log_with_type(mLog, OS_LOG_TYPE_INFO,
                "MIDIFlasher::CreateElement kAudioUnitScope_Group");
            return new SynthGroupElement(this, element, new MidiControls);
            break;
        case kAudioUnitScope_Part:
            os_log_with_type(mLog, OS_LOG_TYPE_INFO,
                "MIDIFlasher::CreateElement kAudioUnitScope_Part");
            return new SynthPartElement(this, element);
            break;
        default:
            os_log_with_type(mLog, OS_LOG_TYPE_INFO,
                "MIDIFlasher::CreateElement default");
            return AUBase::CreateElement(scope, element);
            break;
    }
}

OSStatus MIDIFlasher::GetParameterInfo(AudioUnitScope inScope,
    AudioUnitParameterID inParameterID,
    AudioUnitParameterInfo &outParameterInfo)
{
    char str[LT_AU_MESSAGE_LENGTH] = { 0 };
    parameterIDToString(inParameterID, str);
    os_log_with_type(mLog, OS_LOG_TYPE_INFO,
        "MIDIFlasher::GetParameterInfo ID = %{public}s, "
        "returning kAudioUnitErr_InvalidParameter", str);

    return kAudioUnitErr_InvalidParameter;
}

OSStatus MIDIFlasher::GetPropertyInfo(AudioUnitPropertyID inID,
                                      AudioUnitScope inScope,
                                      AudioUnitElement inElement,
                                      UInt32 &outDataSize,
                                      Boolean &outWritable)
{
    OSStatus status = kAudioUnitErr_InvalidProperty;

    if (inScope == kAudioUnitScope_Global)
    {
        switch (inID)
        {
            case kAudioUnitProperty_CocoaUI:
                outWritable = false;
                outDataSize = sizeof(AudioUnitCocoaViewInfo);
                status =  noErr;
                break;
            case kAudioUnitProperty_MIDIOutputCallbackInfo:
                outDataSize = sizeof(CFArrayRef);
                outWritable = false;
                status = noErr;
                break;
            case kAudioUnitProperty_MIDIOutputCallback:
                outDataSize = sizeof(AUMIDIOutputCallbackStruct);
                outWritable = true;
                status = noErr;
                break;
            default:
                status = kAudioUnitErr_InvalidProperty;
                break;
        }
    }
    else
    {
        status = kAudioUnitErr_InvalidScope;
    }

    char str1[LT_AU_MESSAGE_LENGTH] = { 0 };
    char str2[LT_AU_MESSAGE_LENGTH] = { 0 };
    parameterIDToString(inID, str1);
    statusToString(status, str2);
    os_log_with_type(mLog, OS_LOG_TYPE_INFO,
                    "MIDIFlasher::GetPropertyInfo ID = %{public}s, "
                    "returning %{public}s", str1, str2);

    return status;
}

OSStatus MIDIFlasher::GetProperty(AudioUnitPropertyID inID,
                                  AudioUnitScope inScope,
                                  AudioUnitElement inElement,
                                  void *outData)
{
    OSStatus status = kAudioUnitErr_InvalidProperty;

    if (inScope == kAudioUnitScope_Global) 
    {
        if (inID == kAudioUnitProperty_MIDIOutputCallbackInfo)
        {
            CFStringRef strs[1];
            strs[0] = CFSTR("MIDI Callback");
            CFArrayRef callbackArray =
                CFArrayCreate(NULL, (const void **)strs, 1,
                              &kCFTypeArrayCallBacks);
            *(CFArrayRef *)outData = callbackArray;
            status = noErr;
        }
        else if (inID == kAudioUnitProperty_CocoaUI)
        {
            // Look for a resource in the main bundle by name and type.
            CFBundleRef bundle =
                CFBundleGetBundleWithIdentifier
                    (CFSTR("com.larrymtaylor.au.MIDIFlasher"));
        
            if (bundle == NULL)
            {
                os_log_with_type(mLog, OS_LOG_TYPE_ERROR,
                                 "MIDIFlasher bundle == NULL");
                return -1;
            }
        
            CFURLRef bundleURL =
                CFBundleCopyResourceURL(bundle, CFSTR("MIDIFlasherView"),
                                        CFSTR("bundle"), NULL);

            if (bundleURL == NULL)
            {
                os_log_with_type(mLog, OS_LOG_TYPE_ERROR,
                                 "MIDIFlasher bundleURL == NULL");
                return -1;
            }

            // Name of the main class that implements
            // the AUCocoaUIBase protocol
            CFStringRef className = CFSTR("MIDIFlasher_ViewFactory");
            AudioUnitCocoaViewInfo cocoaInfo = { bundleURL, { className } };
            *((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;
            status = noErr;
        }
    }
    else
    {
        status = kAudioUnitErr_InvalidScope;
    }

    char str1[LT_AU_MESSAGE_LENGTH] = { 0 };
    char str2[LT_AU_MESSAGE_LENGTH] = { 0 };
    parameterIDToString(inID, str1);
    statusToString(status, str2);
    os_log_with_type(mLog, OS_LOG_TYPE_INFO,
                    "MIDIFlasher::GetProperty ID = %{public}s, "
                    "returning %{public}s", str1, str2);

    return status;
}

OSStatus MIDIFlasher::SetProperty(AudioUnitPropertyID inID,
                                  AudioUnitScope inScope,
                                  AudioUnitElement inElement,
                                  const void *inData,
                                  UInt32 inDataSize)
{
    OSStatus status = kAudioUnitErr_InvalidProperty;

    if (inScope == kAudioUnitScope_Global)
    {
        if (inID == kAudioUnitProperty_MIDIOutputCallback)
        {
            if (inDataSize < sizeof(AUMIDIOutputCallbackStruct))
            {
                status = kAudioUnitErr_InvalidPropertyValue;
            }
            else
            {
                AUMIDIOutputCallbackStruct *callbackStruct =
                    (AUMIDIOutputCallbackStruct *)inData;
                mCallbackHelper.SetCallbackInfo
                    (callbackStruct->midiOutputCallback,
                     callbackStruct->userData);
                status = noErr;
            }
        }
        else if (inID == kAudioUnitCustomPropertyUICB)
        {
            if (inDataSize < sizeof(AUMIDIOutputCallbackStruct))
            {
                status = kAudioUnitErr_InvalidPropertyValue;
            }
            else
            {
                AUMIDIOutputCallbackStruct *callbackStruct =
                    (AUMIDIOutputCallbackStruct *)inData;
                mUICB.SetCallbackInfo(callbackStruct->midiOutputCallback,
                                      callbackStruct->userData);
                status = noErr;
            }
        }
    }
    else
    {
        status = kAudioUnitErr_InvalidScope;
    }

    char str1[LT_AU_MESSAGE_LENGTH] = { 0 };
    char str2[LT_AU_MESSAGE_LENGTH] = { 0 };
    parameterIDToString(inID, str1);
    statusToString(status, str2);
    os_log_with_type(mLog, OS_LOG_TYPE_INFO,
                    "MIDIFlasher::SetProperty ID = %{public}s, "
                    "returning %{public}s", str1, str2);

    return status;
}

OSStatus MIDIFlasher::HandleMidiEvent(UInt8 status, UInt8 channel,
                                      UInt8 data1, UInt8 data2,
                                      UInt32 inStartFrame) 
{
    // Snag the MIDI event and then store it in a vector    
    mCallbackHelper.AddMIDIEvent(status, channel, data1, data2, inStartFrame);
    mUICB.AddMIDIEvent(status, channel, data1, data2, inStartFrame);

    return noErr;
}

OSStatus MIDIFlasher::Render(AudioUnitRenderActionFlags &ioActionFlags,
                             const AudioTimeStamp &inTimeStamp,
                             UInt32 inNumberFrames) 
{
    AUInstrumentBase::Render(ioActionFlags, inTimeStamp, inNumberFrames);
    mCallbackHelper.FireAtTimeStamp(inTimeStamp, mLog);
    mUICB.FireAtTimeStamp(inTimeStamp, mLog);

    return noErr;
}
