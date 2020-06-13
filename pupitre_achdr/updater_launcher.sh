#!/bin/bash

# Parameters
USB_KEY_PATH=/media/VIDEO_ACHDR
USB_KEY_PATH=./VIDEO_ACHDR/pack

UPDATER_SCRIPT_NAME="achdr_updater_script.sh"

# Build paths
UPDATER_SCRIPT_PATH=$USB_KEY_PATH/$UPDATER_SCRIPT_NAME

# Check if USB key is accessible
if [[ ! -d "$USB_KEY_PATH" ]]; then
    echo "[E] $USB_KEY_PATH : no such directory"
    exit 1
fi

# Check if there is an updater script to launch
if [[ ! -f "$UPDATER_SCRIPT_PATH" ]]; then
    echo "[E] $UPDATER_SCRIPT_NAME : no such file in usb key"
    exit 1
fi

# Calling the updater and returning the error code
echo "[I] Launching updater script..."
bash "$UPDATER_SCRIPT_PATH" $@
exit $?
