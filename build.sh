#!/bin/bash

DERIVED_DIR=/Users/Larry/Library/Developer/Xcode/DerivedData
BUILD_DIR=${DERIVED_DIR}/MIDIFlasher-*
PRODUCTS_DIR=${DERIVED_DIR}/MIDIFlasher-*/Build/Products/Development

rm -f make.log
touch make.log
rm -rf ${BUILD_DIR}

echo "Building MIDIFlasher" 2>&1 | tee -a make.log

xcodebuild -project MIDIFlasher.xcodeproj clean 2>&1 | tee -a make.log
xcodebuild -project MIDIFlasher.xcodeproj \
    -scheme "MIDIFlasher" build 2>&1 | tee -a make.log
cp -a ${PRODUCTS_DIR} ./MIDIFlasher.xcarchive

