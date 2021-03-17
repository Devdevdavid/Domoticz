# Partitions ESP32

Il est possible de configurer les partitions utilisées par l'ESP en éditant un fichier `*.csv`.
Le fichier par défaut utilisé est localisé ici `~/.platformio/packages/framework-arduinoespressif32/tools/partitions`

Pour utiliser le nouveau fichier dans platformio, on édite le fichier `*.ini` :

```
[base_esp32]
board_build.partitions = src/partition/esp32_SK_1.9M_FS_320k.csv
```

[Tuto partition](https://desire.giesecke.tk/index.php/2018/04/20/change-partition-size-platformio/)

# LD Script ESP8266

Pour les ESP8266, c'est un peu différent, on utilise directement le script du linker.
Les modèles sont ici = `~/.platformio/packages/framework-arduinoespressif8266/tools/sdk/ld`