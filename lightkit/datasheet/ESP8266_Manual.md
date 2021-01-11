# Programmation

La programmation de l'ESP8266 peut s'avérer fastidieuse si l'on a pas tous les éléments en main. Voici un récapitulatif des conditions pour que tout ce passe bien !

## Bootloader

L'ESP8266 dispose d'une mémoire ROM interne qui contient un bootloader. C'est lui qui est lancé lors du démarrage.

Ce bootloader commence par lire l'état des broches pour déterminer le _boot mode_.

Il est possible de consulter le _boot mode_ en se connectant à la liaison série de l'ESP. Le baudrate doit être configuré à **76932 bauds** ([Source](https://www.reddit.com/r/esp8266/comments/51s494/recovering_a_bricked_esp8266_scared_to_brick_my/)). On obtient un message de la sorte : 

```
ets Jan  8 2013,rst cause:2, boot mode:(1,6)
```

## Boot Mode

Le _boot mode_ est indiqué au format `boot mode:(x,y)`. Voici la signification de `x` et `y` ([Source chap. 3.23](https://www.espressif.com/sites/default/files/documentation/Espressif_FAQ_EN.pdf)):

`x`représente ????

`y`représente l'état sur 3 bits des broches `[GPIO15, GPIO0, GPIO2]`

| Valeur de `y` | SD_sel != 3 | SD_sel == 3 |
| :------------- | :----------: | :----------: |
| 0 [0 ,0 ,0 ] | Remapping                           |
| 1 [0 ,0 ,1 ] | UART Boot	                          |
| 2 [0 ,1 ,0 ] | Jump Boot	                          |
| 3 [0 ,1 ,1 ] | FLASH BOOT                         |
| 4 [1 ,0 ,0 ] | SDIO LowSpeed V2 IO	Uart1 Booting |
| 5 [1 ,0 ,1 ] | SDIO HighSpeed V1 IO	Uart1 Booting |
| 6 [1 ,1 ,0 ] | SDIO LowSpeed V1 IO	Uart1 Booting |
| 7 [1 ,1 ,1 ] | SDIO HighSpeed V2 IO	Uart1 Booting |
([Source 1](https://esp8266.ru/esp8266-pin-register-strapping/), [Source 2](https://riktronics.wordpress.com/2017/10/02/esp8266-error-messages-and-exceptions-explained/))

Ici les docs indiquent que l'état des broches impacte l'élément `x`.
Or de ce que j'ai pu tester, mon `x` vaut toujours `1` et c'est mon `y` qui évolue quand je change les GPIO.

Par exemple :

 - `[GPIO15 = 0, GPIO0 = 0, GPIO2 = 0]` donne `(x,y) = (1, 0)`
 - `[GPIO15 = 0, GPIO0 = 0, GPIO2 = 1]` donne `(x,y) = (1, 6)`

Je dirais donc qu'il y a inversion entre `x` et `y`
 
## En pratique

En pratique, il est conseillé de ne pas utiliser les broches concernant le boot pour les applications.

Toutefois, quand il s'agit de l'ESP-01, on est bien obligé de multiplexer ces broches.

Voici donc les conditions à suivre pour passer en mode boot après un reset:

- `GPIO0` doit être à 0
- `GPIO2` doit être à 1
- `GPIO15` doit être à 0 (Non dispo sur l'ESP-01)




 
