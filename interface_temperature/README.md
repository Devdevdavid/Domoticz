# LightKit

LightKit est un logiciel modulaire capable de donner la vie aux cartes électronique de notre cher Pascal.

## Auteurs

* **Pascal RONDANE** - *Concepteur initial*
* **Bastian BOUCHARDON** - *Concepteur initial*
* **David DEVANT** - *Amélioration logicielle* - [Devdevdavid](https://github.com/Devdevdavid)

# Notes pour les développeurs

## Utilisation de platformIO CLI

- `platformio device monitor` : Ouvrir l'interface série pour le debug
- `platformio run -e board_temp_telegram_relay` : Lancer une compilation
- `platformio run --target upload -e board_temp_telegram_relay` : Lancer une compilation suivi d'un upload 

## Utilisation de Clang-Format

Pour appliquer le formattage sur tous les fichiers de `src`:

```bash
clang-format -i src/*.[ch]pp src/*/*.[ch]pp
```

Pour omettre certaines parties du code, on utilise ceci :

```
int formatted_code;
// clang-format off
    void    unformatted_code  ;
// clang-format on
void formatted_code_again;
```

