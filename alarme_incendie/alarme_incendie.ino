//***********************************************
// Telegram test ESP8266
//
//
//
// Test ok le 18/08/20
//***********************************************


#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

#define DEBOUNCE_BUTTON_PERIOD 200
#define RELAY_PULSE_DURATION 3000

// Initialize Wifi connection to the router
char ssid[] = "freebox"; // Network SSID (name) -- Nom du reseau
char password[] = "pascal1961"; // Network password -- Mot de passe

String ThisChat ;
// Initialize Telegram BOT
#define BOTtoken "1283668639:AAHV_cwKanl-HCvwAI7uwn23YBOXo0TJ5Gs"  // your Bot Token (Get from Botfather)

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

const int relayPin = 4;
const int keyPin = 5;
bool ring = false;
uint32_t lastButtonISR = 0;

uint32_t relayCommandTimeout = UINT32_MAX;

int numNewMessages = 0;

void IRAM_ATTR button_isr() {
  // Si on a pas 100ms entre deux interruptions, on ne fait rien
  if ((millis() - lastButtonISR) <= 100) {
    return;
  }

  // Save the moment
  lastButtonISR = millis();
  ring = true;
}

void handleNewMessages(int numNewMessages) {
  String chat_id;
  
  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    ThisChat = chat_id;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/Open") {
      digitalWrite(relayPin, HIGH);   // turn the relay on
      relayCommandTimeout = millis() + RELAY_PULSE_DURATION;
      String keyboardJson = "[[\"/status\"]]";
      bot.sendMessageWithReplyKeyboard(ThisChat, "Door is OPEN\n\nChoose one of the following options", "", keyboardJson, true);
    }

    if (text == "/Ignore") {
      String keyboardJson = "[[\"/status\"]]";
      bot.sendMessageWithReplyKeyboard(ThisChat, "Door opener not used", "", keyboardJson, true);
    }
    if (text == "/status") {
      bot.sendMessage(ThisChat, "Connection OK", "");
    }

    
  
    if (text == "/start") {
      String welcome = "Elektor doorbell via Telegram " + from_name + ".\n";
      welcome += "\n\n";
      welcome += "/Open : to activate door opener\n";
      welcome += "/Ignore : do not open\n";
      welcome += "/status : returns status of doorbell\n";
      
      String keyboardJson = "[[\"/status\"]]";
      bot.sendMessageWithReplyKeyboard(ThisChat, welcome + "\n\nChoose one of the following options", "", keyboardJson, true);
    }
  }

}

void setup() {
 // {
  Serial.begin(115200);
  Serial.println("Ready");
 
  
  pinMode(relayPin, OUTPUT); // initialize digital relayPin as an output.
  delay(10);
  digitalWrite(relayPin, LOW); // initialize pin as off
  pinMode(keyPin, INPUT);
  
  WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  delay(100);
  
  // attempt to connect to Wifi network:
  
  Serial.println("Connecting WiFi: ");
//  Serial.printlnssid);
  
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("\nIP address: ");
  Serial.println(WiFi.localIP());
  
  attachInterrupt(keyPin, button_isr, FALLING);
  //

  client.setInsecure(); // Remplace la fonction HTTPS pour télégram
}

uint8_t debouncing = 0xFF;
uint32_t nextDebounceTick = 0;

void loop() {
    if (ring) {
      ring = false;
      Serial.println("Btn Pushed");
      String keyboardJson = "[[\"/Open\", \"/Ignore\"]]";
      bot.sendMessageWithReplyKeyboard(ThisChat, "Doorbell!\n\nChoose one of the following options", "", keyboardJson, true, true);
    
    }

    if (millis() > relayCommandTimeout) {
      relayCommandTimeout = UINT32_MAX;

      digitalWrite(relayPin, LOW);
    }

  if (millis() > Bot_lasttime + Bot_mtbs)  {
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
     
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
   // delay(1000);
    Bot_lasttime = millis();
    
  }
}
