#!/bin/bash

INSTALL_DIR=/Users/Larry/Library/Audio/Plug-Ins/Components

rm -rf ${INSTALL_DIR}/MIDIFlasher.component
cp -a MIDIFlasher.xcarchive/MIDIFlasher.component ${INSTALL_DIR}

echo "** INSTALL SUCCEEDED **"

