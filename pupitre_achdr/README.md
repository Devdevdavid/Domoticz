# ACHDR Appli
L'appli permet de piloter les pupitres ACHDR. Elle se base sur un script Python.

## Comment incrémenter le numéro de version de l'Appli ACHDR ?

- Commencer par mettre à jour l'entête du fichier `achdr_main_app.py` en ajoutant une ligne dans le ChangeLog
- Mettre à jour le fichier `achdr_soft.config`
- Mettre à jour le numéro de version dans le manuel: `Image/manual.jpg` ([Utiliser PhotoFiltre 7](http://www.photofiltre-studio.com/pf7.htm))

## Comment créer une clé USB compatible avec le logiciel du pupitre ACHDR ?

- Vider le contenu de la clé USB
- Renommer la clé en `VIDEO_ACHDR`
- Copier le dossier `Image` du repository sur la clé. Il contient toutes les images par défaut (mire, loading, etc.)
- Créer un dossier `pack` et copier l'ensemble des fichiers sources du repository (Tous les fichiers sauf le dossier `Image`)
- Ajouter les vidéos, images et présentations à la racine de la clé avec comme nom le numéro du bouton associé (ex: 1.odp, 2.mp4, 3.jpg, 4.odp)
- Enfin, si l'option est activée (Switch à l'arrière du pupitre) c'est le fichier "achdr" qui sera affiché lors du retour à l'acceuil. (ex: achdr.odp ou achdr.jpg)

A la suite de ces étapes, la clé USB est prête à être utilisée avec le pupitre. Voici l'architecture finale obtenue:
```
VIDEO_ACHDR
	├── pack
	│   ├── ACHDRHideCursor.service
	│   ├── ACHDRLauncher.service
	│   ├── achdr_launcher.sh
	│   ├── achdr_main_app.py
	│   ├── ACHDRMainApp.service
	│   ├── ACHDRShutdownButton.service
	│   ├── achdr_soft.config
	│   ├── achdr_updater.sh
	│   └── shutdown_button.sh
	├── Image
	│   ├── achdr2.jpg
	│   ├── achdr_backup.jpg
	│   ├── achdr.jpg
	│   ├── loading.jpg
	│   └── manual.jpg
	├── 1.mp4
	├── 2.jpg
	├── 3.odp
	├── 4.mp4
	└── achdr.mp4
```

# ACHDR Updater
L'updater permet de mettre à jour un pupitre de manière automatique par clé USB.
Le logiciel est capable de repérer le numéro de version installé sur le système et le compare à la version disponible sur la clé USB pour savoir si une mise à jour est nécessaire.

L'updater est composé de plusieurs fichiers :
- `ACHDRLauncher.service` : Ce service permet de lancer le launcher dès que la clé USB est montée par Rasbian
- `achdr_launcher.sh` : Ce fichier doit être placé dans "/home/pi/Scripts". Il permet de lancer l'updater.
- `achdr_updater.sh` : C'est le coeur de l'updater. Ce fichier est placé sur la clé USB et est exécuté par le launcher.
- `achdr_soft.config` : Ce fichier de configuration contient le numéro de version du pack logiciel. Il y a de ce fait deux fichiers comme celui-ci : Le premier est placé dans "/home/pi/Scripts" et contient les informations liées au logiciel installé sur le système. Le second se situe sur la clé USB et contient les informations liées au logiciel disponible sur la clé.

## Installation
Avant de pouvoir profiter de la mise à jour automatique par clé USB, il est nécessaire d'effectuer quelques opérations sur la Raspberry.

- Se connecter en SSH à la raspberry
- Installer les dépendances (Voir plus bas)
- Connecter la clé USB (Elle devrait être montée automatiquement par Raspbian)
- Exécuter la commande suivante pour lancer le launcher depuis la clé USB
```bash
sudo ./media/pi/VIDEO_ACHDR/pack/achdr_launcher.sh
```

Ceci va lancer la mise à jour du système de manière manuelle et de ce fait installer le launcher et le pack logiciel.
À partir de ce moment, dès que la clé VIDEO_ACHDR sera montée, le launcher s'exécutera. S’il est capable de trouver le launcher sur la clé, il l'exécutera.

## Fonctionnement de l'Updater

L'updater est un script bash placé sur la clé USB qui est exécuté par le launcher présent sur le système.
Voici les étapes suivies par le script:
- Lecture du fichier de configuration (`achdr_soft.config`) de la clé pour connaître la version logicielle disponible
- Lecture du fichier de configuration (`achdr_soft.config`) du système pour connaître la version logicielle installée
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

Les logs de l'Updater sont écrits sur la clé de façon à pouvoir les consulter sur un Mac/PC. Le nom du fichier est `VIDEO_ACHDR/pack/achdr_updater.log`

# Dépendances

Ce projet nécessite quelques dépendances pour fonctionner correctement.

- `python` permet de lancer l'application principale
- `unclutter` permet de cacher le curseur lorsque l'on utilise l'affichage de présentation Libre Office
- `wiringpi` permet de lire les boutons et d'allumer les leds
- `omxplayer` permet de lire les vidéos
- `libreoffice` permet de lire les présentations `.odp`
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
Le Markdown c'est chouette : [Basic Syntax](https://www.markdownguide.org/basic-syntax)
