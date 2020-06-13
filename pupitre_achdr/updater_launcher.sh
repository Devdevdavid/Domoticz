#!/bin/bash
# =================================
# Date		: 13 June 2020
# Author	: David DEVANT
# File		: updater_launcher.sh
# Version	: 1.0
# Desc.		: Auto-launch of the
#             updater when USB key
#             is mounted
# =================================

# Parameters
USB_KEY_PATH=/media/VIDEO_ACHDR
USB_KEY_PATH=./VIDEO_ACHDR/pack

UPDATER_SCRIPT_NAME="achdr_updater_script.sh"

# Build paths
UPDATER_SCRIPT_PATH=$USB_KEY_PATH/$UPDATER_SCRIPT_NAME
UPDATER_LOG_FILE=$USB_KEY_PATH/achdr_updater.log

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

# Append the date to the log file
date >> $UPDATER_LOG_FILE

# Explained:
# ---------------
# $@ 							: Give all the arguments of the current script to the updater
# 2>&1 							: Redirect stderr (2) into stdout (1)
# | 							: Redirect updater stdout to the next command
# tee -a <file>				  	: Save input into a file and show the same on screen
# grep -E --color 'pattern|$' 	: colorize patterns

bash "$UPDATER_SCRIPT_PATH" $@ 2>&1 | tee -a $UPDATER_LOG_FILE | grep -E --color '\[E\]|$'
exit $?
