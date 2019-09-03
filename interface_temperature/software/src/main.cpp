//*****************************************
//  Domoticz_interface_DS1820
//  Version 2.3
//*****************************************

/*
 * Read multiple One-Wire DS18B20 probes and publish value on Domoticz with HTTP request
 * Lecture multiple de sonde OneWire DS18B20 et plublication des mesures sur un serveur Domoticz requete HTTP
 * Code adapté - Code adaptated
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>

// Pour une carte ESP8266 | For ESP8266 development board
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

// Configuration
#define NEOPIXEL_BRIGHTNESS     20    // Global Brightness affecting all the colors
#define NEOPIXEL_RED            255
#define NEOPIXEL_GREEN          255
#define NEOPIXEL_BLUE           255
#define MESURE_PERIOD_MS        10*60*1000    // Interval entre deux mesures de température en ms (10 min)
#define CHECK_WIFI_PERIOD_MS    10*1000    // Interval entre deux check wifi en ms (10 sec)

// Pinout
#define ONE_WIRE_BUS            2
#define TEMPERATURE_PRECISION   10
#define NEO_PIXEL_PIN           12

// Constants
#define MAX_SENSOR_SUPPORT      2
#define ADDRESS_LENGTH          sizeof(DeviceAddress)
#define IDX_insideTemp          3
#define IDX_outsideTemp         4
#define NEO_PIXEL_LENGTH        1
#define NEO_PIXEL_INDEX         0

unsigned long previousMillis = 0;
unsigned long lastMillisMesureTemp = 0;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Client Http pour les requetes vers le serveur
HTTPClient http;

// Stockage des adresses des thermometres
byte thermoCount = 0;
DeviceAddress thermoList[MAX_SENSOR_SUPPORT] = {};

// NeoPixel Instance
Adafruit_NeoPixel neoPixel = Adafruit_NeoPixel(NEO_PIXEL_LENGTH, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Parametres WIFI - WiFi settings
#define wifi_ssid               "freebox"
#define wifi_password           "pascal1961"

// Nom d'hôte (pour mDNS)
const char* hostString = "ESP_TEST_TEMP";

// mot de passe pour l'OTA
const char* otapass = "1234";

// Paramètres HTTP Domoticz - HTTP Domoticz settings
const char* host = "192.168.0.32";
const int   port = 8080;

// gestion du temps pour calcul de la durée de la MaJ
unsigned long otamillis;

// Save of neopixel color
uint32_t neopixelColorSave = 0;

void save_visu_led_rgb(void)
{
  neopixelColorSave = neoPixel.getPixelColor(NEO_PIXEL_INDEX);
}

void set_visu_led_rgb(byte red, byte green, byte blue)
{
  neoPixel.setPixelColor(NEO_PIXEL_INDEX, green, red, blue);
  neoPixel.show();
}

void restore_visu_led_rgb(void)
{
  neoPixel.setPixelColor(NEO_PIXEL_INDEX, neopixelColorSave);
  neoPixel.show();
}

/**
 * @brief Configure la mise a jour Over The Air
 */
void confOTA(void)
{
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

void sendToDomoticz(String url)
{
  set_visu_led_rgb(0, NEOPIXEL_GREEN, 0);

  Serial.print("Request: ");
  Serial.print(host);
  Serial.print(" ==> ");
  Serial.println(url);

  http.begin(host, port, url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    Serial.print("[SUCCESS] Domoticz reply:");
    Serial.println(http.getString());
  } else {
    Serial.printf("[ERROR] Bad reply (http code = %d)\n", httpCode);
  }
  http.end();
}

void printTemperature(byte deviceIndex, DeviceAddress deviceAddress){
  float tempC = sensors.getTempC(deviceAddress);

  Serial.printf("[INFO] Sensor %d: ", deviceIndex + 1);
  if (tempC == -127.00) {
    Serial.println("Error getting temperature");
  } else {
    // Format JSON à respecter pour l'API Domoticz - Domoticz JSON API
    // /json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=TEMP
    // https://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Temperature
    String url = "/json.htm?type=command&param=udevice&idx=";
    if (deviceIndex == 0) {
      url += String(IDX_insideTemp);
    } else {
      url += String(IDX_outsideTemp);
    }
    url += "&nvalue=0&svalue=";
    url += String(tempC);
    sendToDomoticz(url);
  }
}

/**
 * @brief Search for new addresses on the OneWire bus
 * Results are saved in addressList
 * @param maxLen: maximum number of address to find
 * @return The number of address found
 */
byte get_onewire_address(DeviceAddress addressList[], byte maxLen)
{
  byte index = 0;

  while (1) {
    if (oneWire.search(addressList[index]) != 1) {
      Serial.printf("Search finished, %d found\n", index);
      oneWire.reset_search();
      break;
    } else {
      // Count this new thermometer
      ++index;
      // Do we have reach the limit ?
      if (index >= maxLen) {
        break;
      }
    }
    delay(100);
  }

  return index;
}

/**
 * @brief Write the thermometers address into thermoList
 * @note It can write 1 or 2 addresses
 */
void get_thermo_address(void)
{
  DeviceAddress addressList[MAX_SENSOR_SUPPORT];

  thermoCount = get_onewire_address(addressList, MAX_SENSOR_SUPPORT);
  if (thermoCount == 1) {
    memcpy(thermoList[0], addressList[0], ADDRESS_LENGTH);
  }
  else if (thermoCount == 2) {
    // The addresses smaller goes in first position
    for (byte index = 0; index < ADDRESS_LENGTH; index++) {
      if (addressList[0][index] > addressList[1][index]) {
        memcpy(thermoList[0], addressList[0], ADDRESS_LENGTH);
        memcpy(thermoList[1], addressList[1], ADDRESS_LENGTH);
      } else if (addressList[0][index] < addressList[1][index]) {
        memcpy(thermoList[0], addressList[1], ADDRESS_LENGTH);
        memcpy(thermoList[1], addressList[0], ADDRESS_LENGTH);
      }
    }
  } else {
    Serial.printf("[ERROR] %d OneWire address found on the bus", thermoCount);
  }
}

//**********************************
// void setup
//**********************************

void setup() {
  Serial.begin(115200);

  // Init neoPixel
  neoPixel.begin();
  neoPixel.setBrightness(NEOPIXEL_BRIGHTNESS);
  set_visu_led_rgb(0, 0, 0);

  // mode Wifi client
  WiFi.mode(WIFI_STA);
  // connexion
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    // impossible de se connecter au point d'accès

    // reboot après 5s
    Serial.println("Erreur connexion Wifi ! Reboot...");
    for (int i = 0; i < 10; i++)
    {
      set_visu_led_rgb(NEOPIXEL_RED, 0, 0);
      delay(250);
      set_visu_led_rgb(0, 0, 0);
      delay(250);
    }

    ESP.restart();
  }

  // configuration OTA
  confOTA();

  // Tout est prêt,on affiche notre IP
  Serial.print("\nAdresse IP: ");
  Serial.println(WiFi.localIP());

  // Locate sensors
  Serial.print("[INFO] Locating devices... ");
  get_thermo_address();
  Serial.printf("Found %d\n", thermoCount);

  sensors.begin();

  // report parasite power requirements
  Serial.print("[INFO] Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  for (byte thermoIndex = 0; thermoIndex < thermoCount; thermoIndex++) {
    // Print device name and address
    Serial.printf("[INFO] Device [%d/%d]: ", thermoIndex + 1, thermoCount);
    for (byte index = 0; index < ADDRESS_LENGTH; index++) {
      if (thermoList[thermoIndex][index] < 0x10) Serial.write('0');
      Serial.print(thermoList[thermoIndex][index], HEX);
      Serial.write(' ');
    }
    Serial.println();

    // Vérifie si les capteurs sont connectés | check and report if sensors are conneted
    if (!sensors.getAddress(thermoList[thermoIndex], thermoIndex)) {
      Serial.printf("    > Unable to find address for Device %d\n", thermoIndex + 1);
      continue;
    }
    // set the resolution to 9 bit per device
    sensors.setResolution(thermoList[thermoIndex], TEMPERATURE_PRECISION);

    // On vérifie que le capteur est correctement configuré | Check that sensor is correctly configured
    Serial.printf("    > Resolution: %d bits\n", sensors.getResolution(thermoList[thermoIndex]));
  }
}

//**********************************
// void loop
//**********************************

void loop() {
  unsigned long currentMillis = millis();

  // gestion OTA
  ArduinoOTA.handle();

  if (currentMillis - previousMillis > CHECK_WIFI_PERIOD_MS) {
    previousMillis = currentMillis;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[ERROR] WiFi not connected !");
      set_visu_led_rgb(NEOPIXEL_RED, 0, 0);
    } else {
      set_visu_led_rgb(0, 0, NEOPIXEL_BLUE);
    }
  }

  if (currentMillis - lastMillisMesureTemp > MESURE_PERIOD_MS) {
    lastMillisMesureTemp = currentMillis;
    Serial.print("[INFO] Requesting temperatures...");
    sensors.requestTemperatures();
    Serial.println("DONE");

    // Save the current color of the neopixel
    save_visu_led_rgb();

    // print the device information
    for (byte thermoIndex = 0; thermoIndex < thermoCount; thermoIndex++) {
      printTemperature(thermoIndex, thermoList[thermoIndex]);
    }

    // Restore neopixel to previous state (After a little delay to correctly see the light)
    delay(100);
    restore_visu_led_rgb();

    Serial.println("");
  }
}
