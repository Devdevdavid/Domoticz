#!/bin/bash

CONFIG_FILE_NAME="achdr_soft.config"

# Get the path of the current file
# https://stackoverflow.com/a/4774063
UPDATER_SCRIPT_PATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

# Build paths
INSTALLED_CONFIG_FILE_PATH=/home/pi/$CONFIG_FILE_NAME
INSTALLED_CONFIG_FILE_PATH=../$CONFIG_FILE_NAME
USBKEY_CONFIG_FILE_PATH=$UPDATER_SCRIPT_PATH/$CONFIG_FILE_NAME

# Move to the updater script
cd $UPDATER_SCRIPT_PATH

is_arg_a_number() {
	regex='^[0-9]+$'
	if [[ $1 =~ $regex ]]; then
		return 1
	else
		return 0
	fi
}

# Parse a simple configuration file
# See https://stackoverflow.com/a/20815951
parse_config_file() {
	shopt -s extglob
	configfile=$1 # set the actual path name of your (DOS or Unix) config file
	tr -d '\r' < $configfile > $configfile.unix
	while IFS='= ' read -r lhs rhs
	do
	    if [[ ! $lhs =~ ^\ *# && -n $lhs ]]; then
	        rhs="${rhs%%\#*}"    # Del in line right comments
	        rhs="${rhs%%*( )}"   # Del trailing spaces
	        rhs="${rhs%\"*}"     # Del opening string quotes
	        rhs="${rhs#\"*}"     # Del closing string quotes
	        export $lhs="$rhs"
	    fi
	done < $configfile.unix

	# Remove tmp file
	rm $configfile.unix

	# Tell no error
	return 0
}

# Read the config file given in params and
# Check the values integrity
read_config() {
	CONFIG_PATH=$1

	# Check if config file exist on the system
	if [[ ! -f "$CONFIG_PATH" ]]; then
		echo "[W] $CONFIG_PATH : no such file"
		return 1
	fi

	# Parse the config file
	parse_config_file $CONFIG_PATH
	if [[ $? -ne 0 ]]; then
		echo "[W] Invalid config data in the installed file"
		return 1
	fi

	# Check params
	is_arg_a_number $VERSION_MAJOR
	if [[ $? -ne 1 ]]; then
		echo "[E] VERSION_MAJOR is not a number: $VERSION_MAJOR"
		return 1
	fi

	is_arg_a_number $VERSION_MINOR
	if [[ $? -ne 1 ]]; then
		echo "[E] VERSION_MINOR is not a number: $VERSION_MINOR"
		return 1
	fi

	return 0
}

# Compare 2 versions composed of a major and
# a minor version number
compare_version() {
	V_MAJOR_CUR=$1
	V_MINOR_CUR=$2
	V_MAJOR_AVAIL=$3
	V_MINOR_AVAIL=$4

	if [[ "$V_MAJOR_AVAIL" -gt "$V_MAJOR_CUR" ]]; then
		return 1		# Update needed
	elif [[ "$V_MAJOR_AVAIL" -eq "$V_MAJOR_CUR" ]]; then
		if [[ "$V_MINOR_AVAIL" -gt "$V_MINOR_CUR" ]]; then
			return 1 	# Update needed
		elif [[ "$V_MINOR_AVAIL" -eq "$V_MINOR_CUR" ]]; then
			return 0	# Up to date
		fi
	fi

	return 2			# Available is depreciated
}

# Read usbkey file
read_config $USBKEY_CONFIG_FILE_PATH
if [[ $? -ne 0 ]]; then
	echo "[E] USBKEY config file is corrupted"
	exit 0
else
	# Save the value
	USBKEY_VMAJOR=$VERSION_MAJOR
	USBKEY_VMINOR=$VERSION_MINOR
fi

# Read installed file
read_config $INSTALLED_CONFIG_FILE_PATH
if [[ $? -ne 0 ]]; then
	echo "[W] Unable to read installed config file due to previous error"

	# Config read failed, continue update
	INSTALLED_VMAJOR=0
	INSTALLED_VMINOR=0
else
	# Save the value
	INSTALLED_VMAJOR=$VERSION_MAJOR
	INSTALLED_VMINOR=$VERSION_MINOR
fi

# Test the version
compare_version $INSTALLED_VMAJOR $INSTALLED_VMINOR $USBKEY_VMAJOR $USBKEY_VMINOR
ret=$?
if [[ "$ret" -eq 2 ]]; then
	echo "[E] The USB key version is depreciated !"
	echo "[E] Installed: v$INSTALLED_VMAJOR.$INSTALLED_VMINOR - USB key: v$USBKEY_VMAJOR.$USBKEY_VMINOR"
	exit 1
elif [[ "$ret" -eq 0 ]]; then
	echo "[I] Installed version is up to date : v$INSTALLED_VMAJOR.$INSTALLED_VMINOR"
	exit 0
elif [[ "$ret" -eq 1 ]]; then
	echo "[I] Update available : v$INSTALLED_VMAJOR.$INSTALLED_VMINOR -> v$USBKEY_VMAJOR.$USBKEY_VMINOR"
else
	echo "[E] compare_version() failed"
	exit 1
fi

echo "[I] Update is starting..."


echo "[I] Update done !"

exit 0