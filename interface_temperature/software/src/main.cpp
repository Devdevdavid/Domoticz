//*****************************************
//
//  Domoticz_interface_DS1820
// 
// Reste a faire un test sous domoticz
// Module OK le 17/05/18 ajout fonction OTA
// IDX= 4 & 5
// Ajout visu led ok le 18/10/18
// Ajout adresse capteurs le 16/07/19
// Passage à PlatformIO le 19/07/19
//
// Version 1.6.10
// Version 1.8.9
// Module Wemos D1 Mini
//*****************************************

/*
 * Read multiple One-Wire DS18B20 probes and publish value on Domoticz with HTTP request
 * Lecture multiple de sonde OneWire DS18B20 et plublication des mesures sur un serveur Domoticz requete HTTP
 * Code adapté - Code adaptated 
 */

#include <OneWire.h>
#include <DallasTemperature.h>
// Pour un Arduino ou ESP32 (le SDK Espressif doit être installé) | For Arduino or ESP32 (Espressif SDK must be installed) 
//#include <WiFi.h>
//#include <HTTPClient.h>

// Pour une carte ESP8266 | For ESP8266 development board
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

// Data wire is plugged into port 4 on the Arduino or ESP32
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 10

#define Led_B 14 //Led Bleu
#define Led_G 12 //Led Green
#define Led_R 16 //Led Rouge
//watchdog timer
const int   watchdog = 10000; // Fréquence d'envoi des données à Domoticz - Frequency of sending data to Domoticz
unsigned long previousMillis = millis();

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Tableaux contenant l'adresse de chaque sonde OneWire | arrays to hold device addresses

DeviceAddress insideThermometer = { 0x28, 0xFF, 0xE1, 0xCD, 0xA1, 0x16, 0x05, 0xDF};
DeviceAddress outsideThermometer = { 0x28, 0xFF, 0x18, 0xE2, 0xA1, 0x16, 0x03, 0xBD};

// Parametres WIFI - WiFi settings
#define wifi_ssid "freebox"
#define wifi_password "pascal1961"
// Nom d'hôte (pour mDNS)
const char* hostString = "espOTAtest2";
// mot de passe pour l'OTA
const char* otapass = "1234";

// Paramètres HTTP Domoticz - HTTP Domoticz settings
const char* host = "192.168.0.32";
const int   port = 8080;
#define IDX_insideTemp    3
#define IDX_outsideTemp   4
HTTPClient http;

// gestion du temps pour calcul de la durée de la MaJ
unsigned long otamillis;

void confOTA() {
  // Port 8266 (défaut)
  ArduinoOTA.setPort(8266);

  // Hostname défaut : esp8266-[ChipID]
  ArduinoOTA.setHostname(hostString);

  // mot de passe pour OTA
  ArduinoOTA.setPassword(otapass);

  // lancé au début de la MaJ
  ArduinoOTA.onStart([]() {
    Serial.println("/!\\ MaJ OTA");
    otamillis=millis();
  });

  // lancé en fin MaJ
  ArduinoOTA.onEnd([]() {
    Serial.print("\n/!\\ MaJ terminee en ");
    Serial.print((millis()-otamillis)/1000.0);
    Serial.println(" secondes");
  });

  // lancé lors de la progression de la MaJ
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progression: %u%%\r", (progress / (total / 100)));
  });

  // En cas d'erreur
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erreur[%u]: ", error);
    switch(error) {
      // erreur d'authentification, mauvais mot de passe OTA
      case OTA_AUTH_ERROR:     Serial.println("OTA_AUTH_ERROR");
                               break;
      // erreur lors du démarrage de la MaJ (flash insuffisante)
      case OTA_BEGIN_ERROR:    Serial.println("OTA_BEGIN_ERROR");
                               break;
      // impossible de se connecter à l'IDE Arduino
      case OTA_CONNECT_ERROR:  Serial.println("OTA_CONNECT_ERROR");
                               break;
      // Erreur de réception des données
      case OTA_RECEIVE_ERROR:  Serial.println("OTA_RECEIVE_ERROR");
                               break;
      // Erreur lors de la confirmation de MaJ
      case OTA_END_ERROR:      Serial.println("OTA_END_ERROR");
                               break;
      // Erreur inconnue
      default:                 Serial.println("Erreur inconnue");
    }
  });

  // Activation fonctionnalité OTA
  ArduinoOTA.begin();
}

void sendToDomoticz(String url){
  digitalWrite(Led_R, LOW);
  digitalWrite(Led_B, HIGH);
  digitalWrite(Led_G, LOW);
  Serial.print("Connecting to ");
  Serial.println(host);
  Serial.print("Requesting URL: ");
  Serial.println(url);
  http.begin(host,port,url);
  int httpCode = http.GET();
    if (httpCode) {
      if (httpCode == 200) {
        String payload = http.getString();
        Serial.println("Domoticz response "); 
        Serial.println(payload);
      }
    }
  Serial.println("closing connection");
  http.end();
}

void printTemperature(String label, DeviceAddress deviceAddress){
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print(label);
  if (tempC == -127.00) {
    Serial.print("Error getting temperature");
  } else {
    // Format JSON à respecter pour l'API Domoticz - Domoticz JSON API 
    // /json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=TEMP
    // https://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Temperature
    if ( label == "Inside : " ) {
      String url = "/json.htm?type=command&param=udevice&idx=";
        url += String(IDX_insideTemp);
        url += "&nvalue=0&svalue=";    
        url += String(tempC); 
      sendToDomoticz(url);
    } else {
      String url = "/json.htm?type=command&param=udevice&idx=";
        url += String(IDX_outsideTemp);
        url += "&nvalue=0&svalue=";    
        url += String(tempC);  
      sendToDomoticz(url);
    }
  }  
}

//**********************************
// void setup
//**********************************

void setup() {
  Serial.begin(115200);

  pinMode(Led_R, OUTPUT);
  pinMode(Led_G, OUTPUT);
  pinMode(Led_B, OUTPUT); 

    // mode Wifi client
  WiFi.mode(WIFI_STA);
  // connexion
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    // impossible de se connecter au point d'accès
    digitalWrite(Led_R, HIGH);
    digitalWrite(Led_G, LOW);
    digitalWrite(Led_B, LOW);
    // reboot après 5s
    Serial.println("Erreur connexion Wifi ! Reboot...");
    delay(5000);
    ESP.restart();
  }

  // configuration OTA
  confOTA();

  // Tout est prêt,on affiche notre IP
  Serial.print("\nAdresse IP: ");
  Serial.println(WiFi.localIP());  
       
  sensors.begin();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // Vérifie sir les capteurs sont connectés | check and report if sensors are conneted 
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
//  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1"); 

  // set the resolution to 9 bit per device
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);

  // On vérifie que le capteur st correctement configuré | Check that ensor is correctly configured
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();

  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(outsideThermometer), DEC); 
  Serial.println();
}

//**********************************
// void loop
//**********************************

void loop() {
  
  unsigned long currentMillis = millis();
  // gestion OTA
  ArduinoOTA.handle();

  if ( currentMillis - previousMillis > watchdog ) {
    previousMillis = currentMillis;
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi not connected !");
      digitalWrite(Led_R, HIGH); // Pas de connection Wifi
      digitalWrite(Led_G, LOW);
      digitalWrite(Led_B, LOW);
      
    } else {  
      Serial.println("Send data to Domoticz");
      digitalWrite(Led_R, LOW);
      digitalWrite(Led_B, LOW);
      digitalWrite(Led_G, HIGH);
    }
  }
  
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");

  // print the device information
  printTemperature("Inside : ", insideThermometer);
  printTemperature("Outside : ", outsideThermometer);
  
  delay(5000);
}