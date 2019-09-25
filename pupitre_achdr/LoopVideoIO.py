# *************************************************
#
# Logiciel pupitre muulimedia ACHDR
# Version 1.1 31/05/19
# Modification des E/S/ Ajout clef USB
# Ajout de la gestion du ventilateur
#
#
# P.Rondane / B.Bourchardon
#
# *************************************************


#-------------------BIBLIOTHEQUES---------------------

import RPi.GPIO as GPIO
import sys
import os
import psutil
import time
from subprocess import Popen

#--------------------VARIABLES------------------------

#Variables bontons poussoir
buttonPins = [13, 15, 29, 31]
#variable interrupteur option
OPTV1V2 = 33
#Variable LEDs
LEDS = [12, 16, 18, 22]
LED_MAV = 32
LED_MAR = 36
#Variable ventilateur
VENTILATEUR = 3
#temporisation clignotement LEDs
TEMPO_START = 1
#temporisation animation fonctionnement normal
TEMPO_ANIM  = 500

cmptAnim = 0
flagAnimCli = False
lastMillis = 0

#Variables Boutons poussoir (bool)
input_state1 = True
input_state2 = True
input_state3 = True
input_state4 = True
#Variables Bouton option
inputV1V2 = True

quit_video = True

timeout = 100

tempoPrint = 0
tempoCheckProcess = 9999 # Grosse valeur pour commencer tout de suite

movieList = [
	"/media/pi/VIDEO_ACHDR/1.mp4",
	"/media/pi/VIDEO_ACHDR/2.mp4",
	"/media/pi/VIDEO_ACHDR/3.mp4",
	"/media/pi/VIDEO_ACHDR/4.mp4",
	"/media/pi/VIDEO_ACHDR/achdr.mp4"
]

# DEFINES
# VIDEO INDEX for movieList
VIDEO_NONE = -1
VIDEO_1 = 0
VIDEO_2 = 1
VIDEO_3 = 2
VIDEO_4 = 3
VIDEO_HOME = 4

player = VIDEO_NONE

buttonStateOld = [0, 0, 0, 0]


#-------------------FONCTIONS-----------------------

def checkProcessRunning():
	# Cherche le processus omxplayer
	for proc in psutil.process_iter():
		if (proc.name() == "omxplayer.bin"):
			return True # Found it !
	
	return False

def ventilateurControler():
	TEMP_MIN = 60 #temperature d'arret du ventilateur
	TEMP_MAX = 70 #temperature de declenchement du ventilateur

	tFile = open('/sys/class/thermal/thermal_zone0/temp')
	temp = float(tFile.read())
	tempC = temp / 1000
	#affichage de la valeur
	print("Temp CPU = " + str(tempC) + " C")

	if (tempC > TEMP_MAX):
		GPIO.output(VENTILATEUR, GPIO.HIGH)
	elif (tempC < TEMP_MIN):
		GPIO.output(VENTILATEUR, GPIO.LOW)
	tFile.close()

def kill_video():
	global player
	os.system('killall omxplayer.bin')
	player = VIDEO_NONE

def launch_video(videoIndex):
	global player
	global tempoCheckProcess
	print("Launching video index {:d}".format(videoIndex))
	kill_video()
	Popen(['omxplayer', '-b', movieList[videoIndex]])
	player = videoIndex
	# Reset de la tempo de check process pour laisser le temps
	# a omxPlayer de démarrer
	tempoCheckProcess = 0;

def read_video_buttons():
	global buttonStateOld

	# Init those variable
	buttonRising = [0] * len(buttonPins)
	buttonFalling = [0] * len(buttonPins)
	buttonLongPress = [0] * len(buttonPins)

	# Read each buttons
	for buttonIndex, buttonPin in enumerate(buttonPins):
		buttonState = GPIO.input(buttonPin)

		# Edge detection
		if (buttonState != buttonStateOld[buttonIndex]):
			if buttonState:
				buttonFalling[buttonIndex] = 1
			else:
				buttonRising[buttonIndex] = 1

		# Long press
		if buttonState:
			buttonLongPress[buttonIndex] += 1
		else:
			buttonLongPress[buttonIndex] = 0

		# Save current button state
		buttonStateOld[buttonIndex] = buttonState

	# Kill video if extrem buttons are pressed
	if buttonRising[0] and buttonRising[3]:
		kill_video()
	else:
		# Launch video of the selected button
		for buttonRisingIndex, buttonRisingState in enumerate(buttonRising):
			if buttonRisingState:
				# Launch corresponding video
				launch_video(buttonRisingIndex)
				break

#--------------------DEMARRAGE--------------------------

GPIO.setmode(GPIO.BOARD) ## Use board pin numbering

# Buttons
for btnPin in buttonPins:
	GPIO.setup(btnPin, GPIO.IN)

GPIO.setup(OPTV1V2, GPIO.IN)

for led in LEDS:
    GPIO.setup(led, GPIO.OUT, initial=GPIO.LOW)

GPIO.setup(LED_MAV, GPIO.OUT, initial=GPIO.LOW)
GPIO.setup(LED_MAR, GPIO.OUT, initial=GPIO.LOW)
GPIO.setup(VENTILATEUR, GPIO.OUT, initial=GPIO.LOW)

# Si la cle USB n'est pas presente
while (not os.path.exists("/media/pi/VIDEO_ACHDR")):
	GPIO.output(LED_MAR, GPIO.HIGH)
	time.sleep(0.1)
GPIO.output(LED_MAR, GPIO.LOW)

GPIO.output(LED_MAV, GPIO.HIGH)

#Test demarrage chenillard + ventilateur
GPIO.output(VENTILATEUR, GPIO.HIGH)
GPIO.output(LEDS[0], GPIO.HIGH)
time.sleep(TEMPO_START)
for i in range(len(LEDS) - 1):
    GPIO.output(LEDS[i + 1], GPIO.HIGH)
    GPIO.output(LEDS[i], GPIO.LOW)
    time.sleep(TEMPO_START)
GPIO.output(LEDS[3], GPIO.LOW)
GPIO.output(VENTILATEUR, GPIO.LOW)

#TEST Option 1 or 2
inputV1V2 = GPIO.input(OPTV1V2)

#-------------------BOUCLE PRINCIPALE--------------------
while True:
	# tempo de la boucle principale
	time.sleep(0.100)

	# Read buttons and launch/kill video if necessary
	read_video_buttons()

	#Si pas de video en cours ou si video home -> animation suivant option
	if player == VIDEO_NONE or player == VIDEO_HOME:
		currentMillis = int(round(time.time() * 1000))
		if (currentMillis - lastMillis) > TEMPO_ANIM:
			lastMillis = currentMillis
			if not inputV1V2:
				for ledIndex, ledPin in enumerate(LEDS):
					if cmptAnim == ledIndex:
						GPIO.output(ledPin, GPIO.HIGH)
					else:
						GPIO.output(ledPin, GPIO.LOW)
				# chenillar
				cmptAnim += 1
				if cmptAnim >= len(LEDS):
					cmptAnim = 0
			else:
				for led in LEDS:
					if flagAnimCli:
						GPIO.output(led, GPIO.HIGH)
					else:
						GPIO.output(led, GPIO.LOW)
				if flagAnimCli:
					flagAnimCli = False
				else:
					flagAnimCli = True
	else:
		# LED management (Show the one selected by the player)
		for ledIndex, ledPin in enumerate(LEDS):
			if player == ledIndex:
				GPIO.output(ledPin, GPIO.HIGH)
			else:
				GPIO.output(ledPin, GPIO.LOW)

	#check de la temperature du systeme
	if (tempoPrint > 100):
		tempoPrint = 0
		ventilateurControler()
	else:
		tempoPrint += 1

	# Tempo a 1 sec
	if (tempoCheckProcess > 10):
		tempoCheckProcess = 0
		#check de l'etat de la video (en cours ou termine)
		if not checkProcessRunning():
			player = VIDEO_NONE
			if (inputV1V2):
				print("Process was not launched ! Relaunching home...")
				launch_video(VIDEO_HOME) # Video d'acceuil
	else:
		tempoCheckProcess += 1


#-------------------FIN DE LA BOUCLE PRINCIPALE----------------------
GPIO.output(LED_MAV, GPIO.LOW)
GPIO.output(VENTILATEUR, GPIO.LOW)
GPIO.cleanup()
