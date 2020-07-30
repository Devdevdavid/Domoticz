#include "global.hpp"
#include "telegram.hpp"
#include "status_led/status_led.hpp"

#ifdef MODULE_TELEGRAM

// Shortcut
#define EN (TELEGRAM_LANG == TELEGRAM_LANG_EN)
#define FR (TELEGRAM_LANG == TELEGRAM_LANG_FR)

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

WiFiClientSecure client;
UniversalTelegramBot bot(TELEGRAM_CONV_TOKEN, client);

void telegram_main()
{
    if (millis() > Bot_lasttime + Bot_mtbs)  {
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        Bot_lasttime = millis();
        while(numNewMessages) {
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
    }
}

void telegram_handle_new_message(int numNewMessages) {
  String chat_id;

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if ((text == "/Marche") || (text == "/On")) {
      digitalWrite(Sortie_1, HIGH);   //alarm activation - activation alarme
      delay(tempo_relay);
      digitalWrite(Sortie_1, LOW);
 #if FR
      bot.sendMessage(ThisChat, "Marche alarme", "");
 #elif (EN)
      bot.sendMessage(ThisChat, "Alarm On", "");
 #endif
    }

   if ((text == "/Arret") || (text == "/Off")) {
      digitalWrite(Sortie_2, HIGH);   // alarm deactivation - déactivation alarme
      delay(tempo_relay);
      digitalWrite(Sortie_2, LOW);
#if FR
      bot.sendMessage(ThisChat, "Arret alarme", "");
#elif (EN)
      bot.sendMessage(ThisChat, "Alarm Off", "");
 #endif

      String keyboardJson = "[[\"/status\"]]";
#if FR
      bot.sendMessageWithReplyKeyboard(ThisChat, "Choisissez l'une des options suivantes", "", keyboardJson, true);
#elif (EN)
      bot.sendMessageWithReplyKeyboard(ThisChat, "Choose one of the following options", "", keyboardJson, true);
#endif
    }

    if ((text == "/Ignore") || (text == "/Skip")) {
      String keyboardJson = "[[\"/status\"]]";
      bot.sendMessageWithReplyKeyboard(ThisChat, "Sortie non utilisee", "", keyboardJson, true);
    }
    if ((text == "/statut") || (text == "/status")) {
      bot.sendMessage(ThisChat, "Connexion OK", "");
    }

    if (text == "/start") {
#if FR
      String welcome = "Interface ES ESP32 alarme " + from_name + ".\n";
      welcome += "\n\n";
      welcome += "/Marche : Mise en marche central d'alarme\n";
      welcome += "/Arret : Arrêt central d'alarme\n";
      welcome += "/Ignore : Aucune action\n";
      welcome += "/statut : Retourne le statut de la connexion\n";
#elif (EN)
      String welcome = "Interface ES ESP32 alarm " + from_name + ".\n";
      welcome += "\n\n";
      welcome += "/On : Starting alarm station\n";
      welcome += "/Off : Shutdown alarm station\n";
      welcome += "/Skip : No action\n";
      welcome += "/status : Return connection state\n";
#endif
      if (ThisChat=="") ThisChat = chat_id;
      bot.sendMessage(ThisChat, welcome, "Markdown");// mise en forme du texte

      String keyboardJson = "[[\"/status\"]]";
#if FR
      bot.sendMessageWithReplyKeyboard(ThisChat, "Choisissez l'une des options suivantes", "", keyboardJson, true);
#elif (EN)
      bot.sendMessageWithReplyKeyboard(ThisChat, "Choose one of the following options", "", keyboardJson, true);
#endif
    }
  }
}

uint8_t telegram_send(String url)
{

}

void telegram_send_temperature(uint8_t sensorID, float degreesValue)
{
#if FR
      bot.sendMessage(ThisChat, "Température: ", "");
#elif (EN)
      bot.sendMessage(ThisChat, "Temperature: ", "");
#endif
}

#endif