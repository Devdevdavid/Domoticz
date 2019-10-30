# Name:     shutdownBtn.sh
# Desc:     Permet de demonter la cle et d'arreter la raspberry quand 
#           on appuis sur le bouton de la GPIO7
# Version:  1.1
# Date:     30 Oct. 2019

# Init 
PIN_BTN=7
PIN_STATE="1"

# Configure
gpio mode $PIN_BTN in

# Wait for it !
while [ $PIN_STATE == "1" ]; do
    sleep 1
    PIN_STATE="$(gpio read $PIN_BTN)"
done

#umount all USB drives
umount /media/pi/*

# Shutdown
shutdown now
