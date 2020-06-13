# ACHDR Updater
L'Updater permet de mettre à jour un pupitre de manière automatique par clé USB.
Le logiciel est capable de repérer le numéro de version installé sur le système et le compare à la version disponible sur la clé USB pour savoir si une mise à jour est nécéssaire.

L'updater est composé de plusieurs fichiers :
- updater_launcher.sh : Ce fichier doit être placé dans "/home/pi/Scripts". Il permet de lancer l'updater.
- ACHDRUpdater.service : Ce service permet de lancer le launcher dès que la clé USB est montée par Rasbian
- achdr_updater_script.sh : C'est le coeur de l'updater. Ce fichier est placé sur la clé USB et est exécuté par le launcher.
- achdr_soft.config : Ce fichier de configuration contient le numéro de version du pack logiciel. Il y a de ce fait deux fichiers comme celui ci : Le premier est placé dans "/home/pi/Scripts" et contient les informations liées au logiciel installé sur le système. Le second se situe sur la clé USB et contient les informations liées au logiciel disponible sur la clé.

## Installation
Cette partie décrit l'installation de l'updater sur le système. Cette opération est nécéssaire une première fois avant de pouvoir utiliser la mise à jour par clé USB.

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

A partir de ce moment, dès que la clé VIDEO_ACHDR sera montée, le launcher s'exécutera. Si il est capable de trouver le launcher sur la clé, il l'exécutera

# Dépendances

Ce projet nécéssite quelques dépendances pour fonctionner correctement.

unclutter permet de cacher le curseur lorsque l'on utilise l'affichage de présentation libre office
```bash
sudo apt install unclutter
```

# Note à Futur Dave
Lorsque l'on est en option (Lancement de la video à Home) alors le mode d'emplois n'est pas lisible car caché par cette vidéo