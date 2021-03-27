# Première connexion avec un module Lightkit

Vous venez de recevoir votre module LightKit et vous souhaitez commencer à l'utiliser au plus vite ? Vous êtes au bon endroit ! Ce tutoriel décrit les étapes à suivre pour installer un module LightKit chez vous.

> Si le titre du paragraphe ne vous concerne pas, passez au suivant.

## Je dispose d'un module antérieur à Avril 2021

Empressez vous de demander une mise à jour logiciel à notre service après vente !

Les modules antérieurs à Avril 2021 ne peuvent être mis à jour qu'avec des outils spécialisés que notre équipe de développeur maîtrise à la perfection.

A partir de la version logicielle v2.1.0, l'utilisateur devient maître de son module toutes les clées lui sont confiées pour qu'il puisse le mettre à jour en toute autonomie.

`- FIN -`

## Je dispose d'un module postérieur à Avril 2021

1. Commencez par alimenter votre module LightKit avec le bloc d'alimentation Micro USB 5V fourni (ou pas).
2. Le module va démarrer en mode _Point D'Accès_. C'est à dire qu'il va "créer" son propre wifi.
3. Prenez votre téléphone ou ordinateur et connectez-vous au réseau wifi du module avec les identifiants suivants :

```
SSID         : LightKit_SSID
Mot de passe : bravo42beta
```

5. Dans votre navigateur préféré, tapez l'URL `http://192.168.4.1/` ou [cliquez ici](http://192.168.4.1/) pour accéder directement à la page de configuration du module.
6. Descendez en bas de la page et cliquez sur le bouton `Configurer le wifi`

Vous êtes désormais prêt à configurer le wifi de votre module. 

## Je souhaite que mon module crée son wifi

1. Dans l'encadré `Mode`, choisissez `Point d'accès`
2. Mettez à jour les différents paramètres suivant votre convenance.

	- SSID : Le nom du réseau wifi crée par le module Lightkit
	- Mot de passe : Le mot de passe nécéssaire pour se connecter au wifi du module
	- IP : L'adresse IP du module, c'est l'adresse qu'il faudra taper dans votre navigateur pour accéder au module
	- Masque de sous-réseau : Le masque doit être en accord avec l'IP. (__Utilisateur expérimenté__)
	- Passerelle : Adresse IP de la passerelle que doit utiliser le module. (__Utilisateur expérimenté__)
	- SSID Caché : Indique si le réseau wifi crée par le module doit être caché. (__Utilisateur expérimenté__)
	- Connexions simultanées : Nombre de client maximum simultané que peut gérer le module (__Utilisateur expérimenté__)

	__ATTENTION__ : Il est fortement conseillé ne ne pas laisser les paramètres par défaut pour le SSID et le mot de passe !

	Valeurs par défaut :

	| Paramètre | Valeur |
	|---|---|
	| SSID | `LightKit_SSID` |
	| Mot de passe | `bravo42beta` |
	| IP | `192.168.4.1` |
	| Masque de sous-réseau | `255.255.255.0` |
	| Passerelle | `192.168.4.254` |
	| SSID caché | `Non` |
	| Nombre de connexions simultanées | `1` |

3. Validez en cliquant sur le bouton `Enregistrer`

Le module va redémarrer avec les nouveaux paramètres. Vous serez déconnecté du réseau, pensez à vous reconnecter.

Vous pouvez commencer à utiliser le module !

`- FIN -`

## Je souhaite utiliser mon module en le connectant à mon réseau wifi existant

1. Choisissez un nom pour votre module. Le nom doit être court et composé uniquement de caractères alphanumériques simples.
2. Dans l'encadré `Mode`, choisissez `Connexion à un réseau existant`
3. Mettez à jour les différents paramètres suivant votre convenance.

	- SSID : Le nom de votre réseau wifi
	- Mot de passe : Le mot de passe nécéssaire pour se connecter à votre réseau wifi
	- Delais de passage en mode AP : Temps en seconde avant de redémarrer en mode point d'accès si le module ne peux pas se connecter avec les identifiants wifi fournis dans sa configuration.
	- Dernière IP: Texte non éditable qui a pour but d'informer l'utilisateur de la dernière adresse IP utilisée par le module.

	> Note : Au démarrage du module, un scan des wifi environnants est effectué. Le résultat de ce scan est affiché en dessous du champ SSID. Vous pouvez cliquer sur un des résultats par charger son nom.

	| Paramètre | Valeur |
	|---|---|
	| SSID | `Aucune` |
	| Mot de passe | `Aucune` |
	| Delais de passage en mode AP | `20` |

4. Validez en cliquant sur le bouton `Enregistrer`


Le module va redémarrer pour appliquer les modifications. Les étapes à suivres dépendent de l'appareil que vous souhaitez utiliser pour vous connecter au module une fois sur votre réseau wifi personnel.

## Je souhaite me connecter à mon module depuis un téléphone Android

Pour se connecter au module Lightkit, deux méthodes sont disponibles :

- `http://<nom du module>.local/`
- `http://<ip du module>/`

Malheuresement, le module LightKit utilise un protocole qui n'est pas supporté par les téléphones Android pour permettre de se connecter avec son nom.

__Les utilisateurs Android n'ont pas le choix__, ils doivent utiliser `http://<ip du module>/` pour se connecter au module LightKit.

Or lors de la première connexion, l'adresse IP attribuée au module par votre box internet n'est pas connu.

La solution consiste ici à suivre la procédure décrite dans le paragraphe suivant en utilisant un appareil compatible pour déterminer l'adresse IP du module et ainsi pouvoir se connecter avec un téléphone Android en tapant l'IP (Ex: `http://192.168.0.42/`)

Cette procédure fastidieuse n'est nécessaire que la première fois.

## Je souhaite me connecter à mon module depuis un appareil compatible

Les appareils compatibles sont les suivants :

- Mac OS
- iOS (iPhone)
- Windows 10
- Linux (avec [avahi](https://avahi.org) d'installé)

Si vous êtes dans le cas d'un appareil compatible, suivez ces étapes :

1. Connectez-vous à votre réseau wifi personnel
2. Dans votre navigateur, tapez l'URL suivante : `http://<nom du module>.local/` en remplacant `<nom du module>` par le nom que vous avez choisi pour le module. (Ex: si le nom choisi est `LightKit`, vous taperez `http://lightkit.local/`)
3. Vous arriverez sur la page de configuration du module.
4. Pour consulter l'adresse IP actuelle du module, accédez à `Configurer le wifi > Dernière IP`

`- FIN -`














