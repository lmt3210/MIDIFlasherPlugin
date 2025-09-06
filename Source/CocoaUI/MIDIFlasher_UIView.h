// 
// MIDIFlasher_UIView.h 
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

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>

#import "LTLog.h"
#import "LTMidi.h"
#import "LTColors.h"

#define kMaximumEvents (1024 * 100)
#define kAudioUnitCustomPropertyUICB 64056

struct LTMIDIControl
{
    int recordEnable;
    int readIndex;
    int writeIndex;
    struct LTMidiEvent recordData[kMaximumEvents];
};

// Note: It is important to rename ALL UI classes when using
// the XCode Audio Unit with Cocoa View template.
// Cocoa has a flat namespace, and if you use the default 
// filenames, it is possible that you will
// get a namespace collision with classes from the 
// cocoa view of a previously loaded audio unit.
// We recommend that you use a unique prefix that 
// includes the manufacturer name and unit name on
// all objective-C source files. You may use an 
// underscore in your name, but please refrain from
// starting your class name with an underscore as these 
// names are reserved for Apple.

@interface MIDIFlasher_UIView : NSView
{
    // Colors
    NSMutableArray *mColors;
    int mColorIndex;
    
    // Main window
    NSView *mView;

    // Menus
    NSMenu *mContextMenu;

    // Other windows
    NSWindow *mHelpWindow;
    NSWindow *mAboutBox;
    
    // Other members
    bool mWindowSet;
    AudioUnit mAU;
    struct LTMIDIControl mMIDIControl;
    NSTimer *mMonitorTimer;
    
    // Logging
    os_log_t mLog;
}

@property (strong) NSMutableArray *mEvents;

// Public functions
- (void)setAU:(AudioUnit)inAU withView:(NSView *)view;

@end
