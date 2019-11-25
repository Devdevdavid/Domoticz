# Name:     shutdownBtn.sh
# Desc:     Permet de demonter la cle et d'arreter la raspberry quand 
#           on appuis sur le bouton de la GPIO7
# Version:  1.1
# Date:     30 Oct. 2019

# Init
PIN_BTN=7
# Wiring number pins
PIN_RED_LED=27
PIN_GREEN_LED=26
PIN_STATE="1"

# Configure
gpio mode $PIN_BTN in

# Wait for it !
while [ $PIN_STATE == "1" ]; do
    sleep 1
    PIN_STATE="$(gpio read $PIN_BTN)"
done
# Configure LED pins
gpio mode $PIN_RED_LED out
gpio mode $PIN_GREEN_LED out

gpio write $PIN_RED_LED 0
gpio write $PIN_GREEN_LED 0
sleep 1

gpio write $PIN_RED_LED 1
#umount all USB drives
umount /media/pi/*

# Shutdown
shutdown now
