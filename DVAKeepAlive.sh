#!/bin/bash

# Disable crash reports
defaults write com.apple.CrashReporter DialogType none

#cd ~/DVA

while [ 1 ]; do

    ./
    
    echo "Restarting Det Virtuelle Akavarie in 5 secs...."
    sleep 5


done;

# Enable crash reports
defaults write com.apple.CrashReporter DialogType prompt