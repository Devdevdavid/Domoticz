# *************************************************
#
# Logiciel pupitre muulimedia ACHDR
# === Version 1.4 25/11/19 - DD
# Cache Curseur sur l'image de home
# Ajout image de loading pour le chargement de libreoffice
# === Version 1.3 16/10/19 - BB
# Ajout image plein écran (OK)
# Modification scrutation des touches
# Modification des E/S/ Ajout clef USB
# Ajout de la gestion du ventilateur
# Ajout d'un test au demarrage pour eviter les problemes sur l'USB
#
#
# P.Rondane / B.Bourchardon / D.Devant
#
# *************************************************


#-------------------BIBLIOTHEQUES---------------------

import RPi.GPIO as GPIO
import sys
import signal
import os
import psutil
import time
import subprocess
from subprocess import DEVNULL, STDOUT
from os import listdir
from os.path import isfile, join
from tkinter import *
# pip3 install Pillow
from PIL import Image, ImageTk

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

# DEFINES
MOUNT_PATH = "/media/pi/"
USB_KEY_PATH = MOUNT_PATH + "VIDEO_ACHDR/"
IMAGE_HOME = USB_KEY_PATH + "Image/achdr.jpg"
IMAGE_LOADING = USB_KEY_PATH + "Image/loading.jpg"
VIDEO_NONE = ""
VIDEO_HOME = "achdr"

currentlyPlaying = VIDEO_NONE

buttonStateOld = [0, 0, 0, 0]


#-------------------FONCTIONS-----------------------
def ventilateur_controler():
	TEMP_MIN = 60 #temperature d'arret du ventilateur
	TEMP_MAX = 70 #temperature de declenchement du ventilateur

	tFile = open('/sys/class/thermal/thermal_zone0/temp')
	temp = float(tFile.read())
	tempC = temp / 1000
	#affichage de la valeur
	#print("Temp CPU = " + str(tempC) + " C")

	if (tempC > TEMP_MAX):
		GPIO.output(VENTILATEUR, GPIO.HIGH)
	elif (tempC < TEMP_MIN):
		GPIO.output(VENTILATEUR, GPIO.LOW)
	tFile.close()

def launch_media(mediaName):
	global currentlyPlaying
	global tempoCheckProcess

	# Turn off
	if mediaName == VIDEO_NONE:
		kill_media()
		return True

	# Liste tous les fichiers de USB_KEY_PATH
	fileNameExtList = [f for f in listdir(USB_KEY_PATH) if isfile(join(USB_KEY_PATH, f))]

	# Pour chaque fichier...
	for fileNameExt in fileNameExtList:
		# Recupere le nom et l'extension
		fileName, fileExt = os.path.splitext(fileNameExt)
		# Compare the name with the button
		if fileName == mediaName:
			# Reconstitution du path complet
			mediaPath = USB_KEY_PATH + fileNameExt
			# Stop all media currently running
			kill_media()
			# Lance le fichier en fonction de l'extension
			if fileExt == ".odp":
				update_home_image(IMAGE_LOADING)
				# Remove Libreoffice lock (.~lock.*.odp# seems not to work)
				subprocess.call(['rm', USB_KEY_PATH + '.~lock.' + fileName + fileExt + '#'], stdout=DEVNULL, stderr=STDOUT)
				# Launch
				print("Launching presentation: " + fileName + fileExt)
				subprocess.Popen(['libreoffice', '--norestore', '--invisible', '--show', mediaPath], stdout=DEVNULL, stderr=STDOUT)
				# Give some time to libreoffice to start before checking process
				tempoCheckProcess = 0
			elif fileExt == ".mp4":
				print("Launching video: " + fileName + fileExt)
				subprocess.Popen(['omxplayer', '-b', mediaPath], stdout=DEVNULL, stderr=STDOUT)
			else:
				print("File extension not supported: \'" + fileExt + "\'")
			# Store the current media name
			currentlyPlaying = mediaName
			# Reset de la tempo de check process pour laisser le temps
			# a omxPlayer de démarrer
			tempoCheckProcess = 0
			# We found the file, leave the function
			return True

	# Error
	print("Couldn't find the media: \"" + mediaName + "\"")
	return False

def kill_media():
	global currentlyPlaying
	update_home_image(IMAGE_HOME)
	subprocess.call(['killall', 'omxplayer.bin'], stdout=DEVNULL, stderr=STDOUT)
	subprocess.call(['killall', 'soffice.bin'], stdout=DEVNULL, stderr=STDOUT)
	currentlyPlaying = VIDEO_NONE

def check_media_running():
	# Cherche le processus omxplayer
	for proc in psutil.process_iter():
		if (proc.name() == "omxplayer.bin"):
			return True # Found it !
		elif (proc.name() == "soffice.bin"):
			return True # Found it !

	return False

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
		kill_media()
	else:
		# Launch video of the selected button
		for buttonRisingIndex, buttonRisingState in enumerate(buttonRising):
			if buttonRisingState:
				# Launch corresponding video
				launch_media(str(buttonRisingIndex + 1)) # (+1: Demarre a 1)
				break

def update_home_image(imgName):
	global labelImage
	img = ImageTk.PhotoImage(Image.open(imgName))
	labelImage.configure(image=img)
	labelImage.image = img

def exit_handler(sig, frame):
	global root
	kill_media()
	root.quit()
	GPIO.cleanup()
	# Restore terminal settings (Popen change them and messep up with new lines)
	subprocess.call(['stty', 'sane'], stdout=DEVNULL, stderr=STDOUT)
	print("Leaving gracefully...")
	sys.exit(0)
#--------------------DEMARRAGE--------------------------

### MOTD
print("=== Starting LoopVideoIO.py ===")

signal.signal(signal.SIGINT, exit_handler)

### Création de la fenetre principale
root = Tk()
# En plein ecran et cache le cursor
root.attributes('-fullscreen', 1)
root.config(cursor="none")
# Creation d'un label pour afficher la photo
labelImage = Label(root)
# "Expand" pour prendre la taille de la fenetre donc de l'ecran
labelImage.pack(fill=BOTH, expand=YES)
# Creation d'un objet PhotoImage pour afficher le fond
update_home_image(IMAGE_HOME)

### Init des GPIO
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD) ## Use board pin numbering

# Buttons
for btnPin in buttonPins:
	GPIO.setup(btnPin, GPIO.IN)

# Option
GPIO.setup(OPTV1V2, GPIO.IN)

# LED
for led in LEDS:
	GPIO.setup(led, GPIO.OUT, initial=GPIO.LOW)
GPIO.setup(LED_MAV, GPIO.OUT, initial=GPIO.LOW)
GPIO.setup(LED_MAR, GPIO.OUT, initial=GPIO.LOW)

# Ventilateur
GPIO.setup(VENTILATEUR, GPIO.OUT, initial=GPIO.LOW)

### Test si il y a eu un probleme avec une cle USB
if (os.path.exists(USB_KEY_PATH + "1")):
	subprocess.call(['rm', '-rf', USB_KEY_PATH], stdout=DEVNULL, stderr=STDOUT)
	subprocess.call(['umount', MOUNT_PATH+"*"], stdout=DEVNULL, stderr=STDOUT)
	subprocess.call(['reboot'], stdout=DEVNULL, stderr=STDOUT)
	exit_handler(None, None)

# Si la cle USB n'est pas presente
while (not os.path.exists(USB_KEY_PATH)):
	GPIO.output(LED_MAR, GPIO.HIGH)
	time.sleep(0.1)

# Active la LED en VERT
GPIO.output(LED_MAR, GPIO.LOW)
GPIO.output(LED_MAV, GPIO.HIGH)

# Test demarrage chenillard + ventilateur
GPIO.output(VENTILATEUR, GPIO.HIGH)
GPIO.output(LEDS[0], GPIO.HIGH)
time.sleep(TEMPO_START)
for i in range(len(LEDS) - 1):
    GPIO.output(LEDS[i + 1], GPIO.HIGH)
    GPIO.output(LEDS[i], GPIO.LOW)
    time.sleep(TEMPO_START)
GPIO.output(LEDS[3], GPIO.LOW)
GPIO.output(VENTILATEUR, GPIO.LOW)

# TEST Option 1 ou 2
inputV1V2 = GPIO.input(OPTV1V2)
if inputV1V2:
	print("Option button is enabled")
else:
	print("Option button is disabled")

#-------------------BOUCLE PRINCIPALE--------------------
while True:
	#update de la fenetre graphique
	root.update()
	# tempo de la boucle principale
	time.sleep(0.100)

	# Read buttons and launch/kill video if necessary
	read_video_buttons()

	# Delay animation
	currentMillis = int(round(time.time() * 1000))
	if (currentMillis - lastMillis) > TEMPO_ANIM:
		lastMillis = currentMillis
		# Si pas de video en cours ou si video home -> animation suivant option
		if currentlyPlaying == VIDEO_NONE or currentlyPlaying == VIDEO_HOME:
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
				if currentlyPlaying == str(ledIndex + 1):
					GPIO.output(ledPin, GPIO.HIGH)
				else:
					GPIO.output(ledPin, GPIO.LOW)

	#check de la temperature du systeme
	if (tempoPrint > 100):
		tempoPrint = 0
		ventilateur_controler()
	else:
		tempoPrint += 1

	# Tempo a 1 sec
	if (tempoCheckProcess > 20):
		tempoCheckProcess = 0
		#check de l'etat de la video (en cours ou termine)
		if not check_media_running():
			if (inputV1V2):
				print("Process was not launched ! Relaunching home...")
				launch_media(VIDEO_HOME) # Video d'acceuil
			else:
				currentlyPlaying = VIDEO_NONE

	else:
		tempoCheckProcess += 1


#-------------------FIN DE LA BOUCLE PRINCIPALE----------------------
