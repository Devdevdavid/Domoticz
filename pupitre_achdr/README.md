# ACHDR Appli
TODO
# ACHDR Updater
L'updater permet de mettre à jour un pupitre de manière automatique par clé USB.
Le logiciel est capable de repérer le numéro de version installé sur le système et le compare à la version disponible sur la clé USB pour savoir si une mise à jour est nécessaire.

L'updater est composé de plusieurs fichiers :
- `updater_launcher.sh` : Ce fichier doit être placé dans "/home/pi/Scripts". Il permet de lancer l'updater.
- `ACHDRUpdater.service` : Ce service permet de lancer le launcher dès que la clé USB est montée par Rasbian
- `achdr_updater_script.sh` : C'est le coeur de l'updater. Ce fichier est placé sur la clé USB et est exécuté par le launcher.
- `achdr_soft.config` : Ce fichier de configuration contient le numéro de version du pack logiciel. Il y a de ce fait deux fichiers comme celui-ci : Le premier est placé dans "/home/pi/Scripts" et contient les informations liées au logiciel installé sur le système. Le second se situe sur la clé USB et contient les informations liées au logiciel disponible sur la clé.

## Installation
Cette partie décrit l'installation de l'updater sur le système. Cette opération est nécessaire une première fois avant de pouvoir utiliser la mise à jour par clé USB.

Copiez les fichiers sur le système:
```bash
sudo cp <chemin-source-du-service> /etc/systemd/system/ACHDRUpdater.service
sudo cp <chemin-source-du-launcher> /home/pi/Scripts/updater_launcher.sh
```

Activez le service et lancez-le:
```bash
sudo systemctl enable ACHDRUpdater
sudo systemctl start ACHDRUpdater
```

À partir de ce moment, dès que la clé VIDEO_ACHDR sera montée, le launcher s'exécutera. S’il est capable de trouver le launcher sur la clé, il l'exécutera.

## Fonctionnement de l'Updater

L'updater est un script bash placé sur la clé USB qui est exécuté par le launcher présent sur le système.
Voici les étapes suivies par le script:
- Lecture du fichier de configuration de la clé pour connaître la version logicielle disponible
- Lecture du fichier de configuration du système pour connaître la version logicielle installée
- Comparaison des deux numéros de version. Si le logiciel n'est pas installé sur le système, le numéro de version est défini à v0.0.
- Si le système est à jour ou si la version de la clé est obsolète, le script s'arrête
- Si une mise à jour est disponible alors on commence la routine de mise à niveau.

Voici à présent le processus mis en place pour mettre le système à jour:
- L'Updater va désinstaller la version actuelle à l'aide de l'Updater qui a permis l'installation. Ainsi, une même version logicielle est installée et désinstallé par une même version de l'Updater. Ceci permet d'éviter les problèmes de compatibilité ascendante de l'updater.
- Chaque service est désactivé puis supprimé
- Les scripts sont supprimés
- Seul le launcher de l'updater est conservé. Ceci permet de réutiliser la mise à jour par clé USB même à la suite d'une désinstallation.
- L'updater présent sur la clé reprend ensuite la main pour réaliser l'installation de la nouvelle version
- Les scripts sont copiés de la clé vers le système.
- Les services sont installés et démarrés

Les logs de l'Updater sont écrits sur la clé de façon à pouvoir les consulter sur un Mac/PC.

# Dépendances

Ce projet nécessite quelques dépendances pour fonctionner correctement.

- `python` permet de lancer l'application principale
- `unclutter` permet de cacher le curseur lorsque l'on utilise l'affichage de présentation Libre Office
- `wiringpi` permet de lire les boutons et d'allumer les leds
- `omxplayer` permet de lire les vidéos
- `libreoffice` permet de lire les présentations `.ODT`
- `feh` permet d'afficher des images en plein écran
- `python3-tk` permet de créer des interfaces graphiques
- `python3-rpi.gpio` permet de contrôler les GPIO depuis Python

```bash
sudo apt install python unclutter wiringpi omxplayer libreoffice feh python3-tk python3-dev python3-rpi.gpio
```

Il faut aussi installer quelques librairies pythons:
```bash
pip3 install RPi.GPIO psutil Pillow
```

# Note à Futur Dave
Lorsque l'on est en option (Lancement de la vidéo à Home) alors le mode d'emploi n'est pas lisible car caché par cette vidéo.
