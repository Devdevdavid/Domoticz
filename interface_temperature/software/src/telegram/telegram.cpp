#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include "global.hpp"
#include "telegram.hpp"

#ifdef MODULE_TELEGRAM

// EXTERNS
extern uint32_t tick;

// STATIC
static uint32_t nextCheckTick = 0;
static String linkedChat;
static WiFiClientSecure wiFiClientSecure;
static UniversalTelegramBot TBot(TELEGRAM_CONV_TOKEN, wiFiClientSecure);

static void telegram_msg_send_motd(void)
{
#if TELEGRAM_LANG == TELEGRAM_LANG_FR
	String welcome = "Interface ES ESP32 alarme.\n";
	welcome += "\n\n";
	welcome += "/Marche : Mise en marche central d'alarme\n";
	welcome += "/Arret : Arrêt central d'alarme\n";
	welcome += "/Ignore : Aucune action\n";
	welcome += "/statut : Retourne le statut de la connexion\n";
#elif TELEGRAM_LANG == TELEGRAM_LANG_EN
	String welcome = "Interface ES ESP32 alarm.\n";
	welcome += "\n\n";
	welcome += "/On : Starting alarm station\n";
	welcome += "/Off : Shutdown alarm station\n";
	welcome += "/Skip : No action\n";
	welcome += "/status : Return connection state\n";
#endif
	TBot.sendMessage(linkedChat, welcome, "Markdown");// mise en forme du texte
}

static void telegram_handle_new_message(telegramMessage * message) {
	// Check command received
	if ((message->text == "/Arret") || (message->text == "/Off")) {
		String keyboardJson = "[[\"/status\"]]";
		TBot.sendMessageWithReplyKeyboard(linkedChat, TG_MSG_CHOOSE_OPTION, "", keyboardJson, true);

		// Unlink client
		linkedChat = "";
	}
	else if ((message->text == "/statut") || (message->text == "/status")) {
		TBot.sendMessage(linkedChat, TG_MSG_CONNECTION_OK, "");
	}
	else if (message->text == "/start") {
		telegram_msg_send_motd();

		// Link the new client
		linkedChat = String(message->chat_id);

	    String keyboardJson = "[[\"/status\"]]";
	    TBot.sendMessageWithReplyKeyboard(linkedChat, TG_MSG_CHOOSE_OPTION, "", keyboardJson, true);
	}
}

// FUNCTION

void telegram_init(void)
{
	nextCheckTick = 0;

	// This is the simplest way of getting this working
  	// if you are passing sensitive information, or controlling
  	// something important, please either use certStore or at
  	// least client.setFingerPrint
	// https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/issues/104#issuecomment-485255312
	wiFiClientSecure.setInsecure();
}

void telegram_main(void)
{
	int msgNumber;

	if (tick >= nextCheckTick)  {
		nextCheckTick = tick + TELEGRAM_CHECK_PERIOD_MS;
		log_info("TBot Update %d", tick);

		do {
			msgNumber = TBot.getUpdates(TBot.last_message_received + 1);
			for (int i = 0; i < msgNumber; i++) {
				log_info("Handling TBot message %d: %s", i, TBot.messages[i].text.c_str());
				telegram_handle_new_message(&TBot.messages[i]);
			}
		} while (msgNumber);
	}
}

void telegram_send_msg_temperature(uint8_t sensorID, float degreesValue)
{
	// We didn't start yet
	if (linkedChat == "") {
		return;
	}

	String msg = String(sensorID) + "] " + TG_MSG_TEMPERATURE_IS + String(degreesValue) + "'C";
	TBot.sendMessage(linkedChat, msg, "");
	log_info("Sending \"%s\" to %s", msg.c_str(), linkedChat.c_str());
}

#endif