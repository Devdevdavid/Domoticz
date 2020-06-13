# Name:     shutdown_button.sh
# Desc:     Permet de demonter la cle et d'arreter la raspberry quand
#           on appuis sur le bouton de la GPIO7
# Version:  1.2
# Date:     13-06-2020

# Pinout (Wiring Pi notation)
PIN_BTN=7
PIN_RED_LED=27
PIN_GREEN_LED=26

# Check if the command is supported
# This avoid auto-stopping the RPi when
# this script is installed and the
# command is not
command -v gpio > /dev/null
if [[ $? -ne 0 ]]; then
	echo "[E] \"Wiring Pi\" is not installed on the system"
	echo "[I] Use \"sudo apt install wiringpi\" to resolve this"
	return 1
fi

# Configure
gpio mode $PIN_BTN in

# Wait for it !
PIN_STATE="1"
while [ $PIN_STATE == "1" ]; do
    sleep 1
    PIN_STATE="$(gpio read $PIN_BTN)"
done

# Configure LED pins
gpio mode $PIN_RED_LED out
gpio mode $PIN_GREEN_LED out

# Animation
gpio write $PIN_RED_LED 0
gpio write $PIN_GREEN_LED 0
sleep 1
gpio write $PIN_RED_LED 1

# umount all USB drives
umount -q /media/pi/*

# Shutdown
shutdown now
