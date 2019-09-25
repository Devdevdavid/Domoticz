# Name:     shutdownBtn.sh
# Desc:     Permet d'arreter la raspberry quand 
#           on appuis sur le bouton de la GPIO7
# Version:  1.0
# Date:     25 Sept. 2019

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

# Shutdown
shutdown now