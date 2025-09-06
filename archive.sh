#!/bin/bash

VERSION=$(cat MIDIFlasher.xcodeproj/project.pbxproj | \
          grep -m1 'MARKETING_VERSION' | cut -d'=' -f2 | \
          tr -d ';' | tr -d ' ')
ARCHIVE_DIR=/Users/Larry/Library/Developer/Xcode/Archives/CommandLine

rm -rf ${ARCHIVE_DIR}/MIDIFlasher-v${VERSION}.xcarchive
cp -a MIDIFlasher.xcarchive ${ARCHIVE_DIR}/MIDIFlasher-v${VERSION}.xcarchive
cd MIDIFlasher.xcarchive && \
    zip -rq ../MIDIFlasher-v${VERSION}.zip MIDIFlasher.component
cd ..

echo "** ARCHIVE SUCCEEDED **"

