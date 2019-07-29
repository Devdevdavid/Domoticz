//----------------------------------------------------------
//
// Gestion barre à leds multicolore
// 
// Test ok 27/12/16
// Version V1.0
// 
// 28/01/01/16 => Plilotage 180 led ou 3 metres
// 1 metre/6 led = 3.5A
// 192.168.4.1
// Port serie 115.200 
//
// LED_COUNT =  180 led
//
// P.Rondane/B.Bouchardon
// Compilateur version 1.6.10
//----------------------------------------------------------

#include <WS2812FX.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

const char *ssid = "Led_RGB";
const char *password = "Tours2016";

//flag pour le choix d'effet
bool flag_effet = true;

ESP8266WebServer server(80);

//chaine de caractère contenant la page HTML
String web_page = "";

//Nombre de WS2812
//#define LED_COUNT 78
#define LED_COUNT 180
// Pin sortie données
#define LED_PIN 2

//Instanciation de l'objet représentant le ruban
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long previousMillis = 0;
const long interval = 10000; 

int effet = -1;

// fonction des parametres de la page web HTML
void parametres(){
  //recuperation des arguments = parametres
  int lum = server.arg("Lum").toInt();
  int effet = server.arg("effet").toInt();

  //parametre de luminosite
  ws2812fx.setBrightness(lum);
  Serial.print("WEB - Lum = ");
  Serial.println(lum);

  //parametre de choix d'effet
  if(effet == 100){ //mode demo
    flag_effet = true;
    Serial.println("WEB - effet = demo");
  }
  else if(effet > 44){ //depassement nb effet
    ws2812fx.setMode(44);
    Serial.println ("WEB - error = mode 44");
  }
  else{ // choix effet
    flag_effet = false;
    ws2812fx.setMode(effet);
    Serial.print("WEB - effet = ");
    Serial.println(effet);
  }

  //renvoi de la page HTML
  server.send(200, "text/html", web_page);
}


//fonction page HTML
void handleRoot() {
  web_page = "Content-Type: text/html;charset=UTF-8";
  web_page = "<HTML>\n<HEAD>";
  web_page = "</HEAD><BODY>";
  web_page = "<h1>Commande guirlande RGB V1.0</h1>";
  web_page += " <p>";
  web_page += " <h2>Choix de la luminosite</h2>";
  web_page += " <form action=\"/led\" method=get>";
  web_page += "     <select name=\"Lum\">";
  web_page += "         <option value=\"0\">0%</option>";
  web_page += "         <option selected value=\"64\">25%</option>"; // valeur par defaut
  web_page += "         <option value=\"127\">50%</option>";
  web_page += "         <option value=\"191\">75%</option>";
  web_page += "         <option value=\"255\">100%</option>";
  web_page += "     </select>";
  web_page += " </p>";
  web_page += " <p>";
  web_page += " <h2>Choix de l'effet (100 = demo, max = 44)</h2>";
  web_page += " <input type=\"number\" name=\"effet\" maxlength=\"3\" size=\"6\" /><br />";
  web_page += " </p>";
  web_page += "<br><input type=\"submit\" value=\"Envoi\" /></form>";
  web_page += "</BODY></HTML>";

  server.send(200, "text/html", web_page);
}

//Configuration
void setup() {
// Initialisation
  ws2812fx.init();
//Luminosité sur 1/4 du maximun (255)
  ws2812fx.setBrightness(25);
// Réglage de la vitesse des effets  
  ws2812fx.setSpeed(500);
//Un arc en cile glissant  
  ws2812fx.setColor(255, 255, 0);
  ws2812fx.setMode(FX_MODE_CHASE_RAINBOW);
// Demarrage  
  ws2812fx.start();
  Serial.begin(115200);

  //Initialisation de la connexion wifi
  WiFi.softAP(ssid, password);

  
  //lancement de la page web
  server.on("/", handleRoot);
  //detection des parametres
  server.on("/led", parametres);

  
  server.begin();
  Serial.println("Serveur accessible");
}

void loop() {

  server.handleClient();
  
//Rafraichissement de l'animation  
  ws2812fx.service();
  if(flag_effet){
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      ws2812fx.stop();
      effet++;
      if(effet >= MODE_COUNT) effet = 0;
      Serial.println(ws2812fx.getModeName(effet));
      ws2812fx.setMode(effet);
      ws2812fx.start();
      previousMillis = currentMillis;
    }
  }
}
