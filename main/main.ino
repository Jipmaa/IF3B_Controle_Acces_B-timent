#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Stepper.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>          // Cette bibliothèque permet à l'ESP32 de se connecter au réseau WiFi.
#include <PubSubClient.h>  // Cette bibliothèque vous permet d'envoyer et de recevoir des messages MQTT.


//Oled
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
//Keypad
#define RX_PIN 17  // Connecté à TX du keypad
#define TX_PIN 16  // Connecté à RX du keypad
// Declaration for SSD1306 display connected using I2C
#define OLED_RESET -1  // Reset pin
#define SCREEN_ADDRESS 0x3C
//Led Stick
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif
#define LED_PIN 33 //au lieu de 19
#define LED_COUNT 8
#define RST_PIN 4
#define SS_PIN 5

// "Variables" globales
const int relay = 32;
const int buzzer = 13;
const int ledRed = 33;
const int stepsPerRevolution = 2038;
int red = 255;
int green = 255;
int blue = 255;
String lastMessageOLED = "";
String lastStateDoor = "close";
unsigned int freq_tone = 200;
unsigned long time_tone = 200;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins) = oled
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// rfid
MFRC522 mfrc522(SS_PIN, RST_PIN);
// Init array that will store new NUID
byte nuidPICC[4];
// Stepper
Stepper myStepper(stepsPerRevolution, 14, 26, 27, 25);
//Keypad
HardwareSerial TRANS_SERIAL(1);  // Utiliser le port UART1
String codeCorrect = "5190";
String codeCorrectEmploye = "1234";
String codeSaisi = "";
//LedStocl
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);
//Wifi
WiFiClient espClientTHUNE;                 // Initialiser la bibliothèque client wifi (connexion WiFi)
PubSubClient clientTHUNE(espClientTHUNE);  // Créer un objet client MQTT (connexion MQTT)

const char* ssid = "Partage de co";                //Nom du wifi
const char* password = "Tomates08";                //Mot de passe du wifi
const char* mqtt_server = "mqtt.ci-ciad.utbm.fr";  //Nom du serveur
long lastMsg = 0;

void setup() {
  Serial.begin(9600);  // Initialisation du port série
  while (!Serial)
    ;
  delay(100);
  Serial.println("Debug: Début de setup.");

  pinMode(ledRed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  myStepper.setSpeed(8);

  //rfid
  SPI.begin();                        // Init SPI bus
  mfrc522.PCD_Init();                 // Init MFRC522
  delay(100);                         // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  // Initialisation de l'écran OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Erreur : OLED non détecté."));
    while (true)
      ;  // Boucle infinie si l'OLED échoue
  }
  affichageScan(0, "a");  // Affichage par défaut

  // Initialisation du clavier
  TRANS_SERIAL.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("Debug: Clavier initialisé.");

  // Initialisation de la bande LED
  strip.begin();
  strip.show();
  strip.setBrightness(50);
  colorWipe(strip.Color(green, red, blue), 10);  // Couleur blanche par défaut


  setup_wifi();
  clientTHUNE.setServer(mqtt_server, 1883);
  clientTHUNE.setCallback(callback);
  Serial.println("Debug: Fin de setup.");
}



void loop() {
  checkingData();
  recupDataKeyboard();
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  Serial.println("Erreur 3333333333333333333333333");
  String UID = "";

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    UID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    UID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  UID.toUpperCase();

  if (UID.substring(1) == "36 92 8B 44" || ((UID.substring(1) == "8B 7E BD 22" || UID.substring(1) == "BA FC A6 16" || 
  UID.substring(1) == "39 77 91 6D") && codeSaisi == codeCorrectEmploye)) {

    sendData(2);
    affichageScan(1, "Employe\nreconnu");
    delay(2000);
    ouverture();

  } else if (UID.substring(1) == "8B 7E BD 22" || UID.substring(1) == "BA FC A6 16" || UID.substring(1) == "39 77 91 6D") {
    sendData(1);
    ouverture();
  }  else {
    affichageScan(2, "Badge non autorise");
    erreur();
  }
}

void ouverture() {
  codeSaisi = "";
  Serial.println("test beginning condition");

  affichageScan(1, "Ouverture");  //oled

  //https://javl.github.io/image2cpp/?pseSrc=pgEcranOledArduino
  //serrure
  digitalWrite(relay, HIGH);  //serrure "éjecté"

  digitalWrite(ledRed, LOW);

  tone(buzzer, 1000, 200);
  delay(50);
  tone(buzzer, 250, 200);

  //moteur
  Serial.println("Ouverture");
  clientTHUNE.publish("esp32/maison/doorState", "En ouverture");
  myStepper.step(-stepsPerRevolution * 1.2);
  clientTHUNE.publish("esp32/maison/doorState", "Ouverte");
  green = 255;
  red = 0;
  blue = 0;
  for (int i = 0 ; i < 5 ; i++) {
    colorWipe(strip.Color(green, red, blue), 100);
    green = 0;
    red = 255;
    colorWipe(strip.Color(green, red, blue), 100);
    red = 0;
    blue = 255;
    colorWipe(strip.Color(green, red, blue), 100);
    blue = 0;
    green = 255;
    colorWipe(strip.Color(green, red, blue), 100);
  }
  blue = 255;
  red = 255;
  colorWipe(strip.Color(green, red, blue), 10);


  affichageScan(1, "Fermeture imminente");

  for (int j = 0; j < 2; j++) {
    digitalWrite(ledRed, HIGH);
    tone(buzzer, 250, 200);
    delay(500);
    digitalWrite(ledRed, LOW);
    tone(buzzer, 250, 200);
    delay(500);
  }

  digitalWrite(relay, LOW);
  tone(buzzer, 250, 1500);
  affichageScan(1, "Fermeture");

  // 1 rotation counterclockwise:
  Serial.println("Fermeture");
  clientTHUNE.publish("esp32/maison/doorState", "En fermeture");
  myStepper.step(stepsPerRevolution * 1.2);
  clientTHUNE.publish("esp32/maison/doorState", "Fermée");
  delay(1000);

  colorWipe(strip.Color(255, 255, 255), 1);  // White

  affichageScan(0, "a");
}

void erreur() {

  sendData(0);

  tone(buzzer, 1000, 200);
  delay(50);
  tone(buzzer, 250, 1000);

  for (int j = 0; j < 5; j++) {
    digitalWrite(ledRed, HIGH);
    delay(500);
    digitalWrite(ledRed, LOW);
    delay(500);
  }

  tone(buzzer, 250, 200);

  affichageScan(0, "a");
}


// Affiche un message par défaut si x = FALSE
void affichageScan(int x, String msg) {

  if (x == 1) {

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5, 30);

    // Display static text
    display.println(msg);
    display.display();
    display.stopscroll();

  } else if (x == 0) {

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5, 30);

    // Display static text
    display.println("Scan badge");
    display.display();
    display.stopscroll();

  } else if (x == 2) {
    display.clearDisplay();
    display.setTextColor(INVERSE);
    display.setCursor(5, 30);
    display.println(msg);
    display.startscrollleft(0x00, 0x07);
    display.display();
  }
}


// Keypad
void recupDataKeyboard() {
  while (TRANS_SERIAL.available()) {
    uint8_t data = TRANS_SERIAL.read();
    switch (data) {
      case 0xE1:  // "1"
        Serial.println("1");
        ajouterTouche('1');
        tone(buzzer, 750, 200);
        break;
      case 0xE2:  // "2"
        Serial.println("2");
        ajouterTouche('2');
        tone(buzzer, 800, 200);
        break;
      case 0xE3:  // "3"
        Serial.println("3");
        ajouterTouche('3');
        tone(buzzer, 850, 200);
        break;
      case 0xE4:  // "4"
        Serial.println("4");
        ajouterTouche('4');
        tone(buzzer, 900, 200);
        break;
      case 0xE5:  // "5"
        Serial.println("5");
        ajouterTouche('5');
        tone(buzzer, 950, 200);
        break;
      case 0xE6:  // "6"
        Serial.println("6");
        ajouterTouche('6');
        tone(buzzer, 1000, 200);
        break;
      case 0xE7:  // "7"
        Serial.println("7");
        ajouterTouche('7');
        tone(buzzer, 1050, 200);
        break;
      case 0xE8:  // "8"
        Serial.println("8");
        ajouterTouche('8');
        tone(buzzer, 1100, 200);
        break;
      case 0xE9:  // "9"
        Serial.println("9");
        ajouterTouche('9');
        tone(buzzer, 1150, 200);
        break;
      case 0xEA:  // "*"
        Serial.println("*");
        Serial.println("Effacement du code saisi.");
        tone(buzzer, 1200, 200);
        affichageScan(0, "a");
        digitalWrite(ledRed, HIGH);
        delay(500);
        digitalWrite(ledRed, LOW);
        codeSaisi = "";  // Réinitialise le code
        break;
      case 0xEB:  // "0"
        Serial.println("0");
        ajouterTouche('0');
        tone(buzzer, 1250, 200);
        break;
      case 0xEC:  // "#"
        Serial.println("#");
        tone(buzzer, 1300, 200);
        verifierCode();  // Vérifie le code saisi
        break;
      default:
        Serial.println("Touche inconnue.");
    }
  }
}


void ajouterTouche(char touche) {
  codeSaisi += touche;  // Ajoute la touche au code saisi
  affichageCode();
}


void verifierCode() {
  Serial.println("Code entré : ");
  Serial.print(codeSaisi);

  if (codeSaisi == codeCorrect) {

    Serial.println("Code correct ! Ouverture...");
    affichageScan(1, "Code bon");
    delay(500);
    sendData(1);
    ouverture();

  } else {

    Serial.println("Code incorrect ! Accès refusé.");
    affichageScan(1, "Code\nincorrect");
    erreur();
    delay(500);
  }
}



void affichageCode() {

  Serial.print("Longueur du code : ");
  Serial.println(codeSaisi.length());
  Serial.print("Code saisi : ");
  Serial.println(codeSaisi);

  if (codeSaisi.length() < 5) {  // à remettre 5 ici

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(25, 30);

    for (int i = 0; i < codeSaisi.length(); i++) {
      
      display.print("*");
    }

    display.display();
  }

  else {
    affichageScan(1, "Code\nincorrect");
    codeSaisi = "";
    erreur();
  }
}



// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  green = 255;
  red = 0;
  blue = 0;
  for (int i = 0 ; i < 5 ; i++) {
    colorWipe(strip.Color(green, red, blue), 100);
    green = 0;
    red = 255;
    colorWipe(strip.Color(green, red, blue), 100);
    red = 0;
    blue = 255;
    colorWipe(strip.Color(green, red, blue), 100);
    blue = 0;
    green = 255;
  }
  red = 255;
  blue = 255;
  colorWipe(strip.Color(green, red, blue), 10);
}


void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    strip.show();                                //  Update strip to match
    delay(wait);                                 //  Pause for a moment
  }
}



void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


// Application des requêtes MQTT reçues via nodered
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.println(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/maison/door") {
    if (messageTemp == "onDoor") {  
    clientTHUNE.publish("esp32/maison/doorState", "En ouverture");
      digitalWrite(relay, HIGH);  //serrure "éjecté"
      myStepper.step(-stepsPerRevolution * 1.2);  // Ouvrir la porte
      digitalWrite(relay, LOW); 
      clientTHUNE.publish("esp32/maison/doorState", "Ouverte");
      Serial.println("Porte ouverte !");
    } else if (messageTemp == "offDoor") {
      clientTHUNE.publish("esp32/maison/doorState", "En fermeture");
      myStepper.step(stepsPerRevolution * 1.2);  // Fermer la porte
      clientTHUNE.publish("esp32/maison/doorState", "Fermée");
      Serial.println("Porte fermee !");
    } else {
      Serial.println("Problème messageTemp pour Moteur");
    }
  }


  if (String(topic) == "esp32/maison/ledRed") {
    if (messageTemp == "onLed") {
      digitalWrite(ledRed, HIGH);
    } else if (messageTemp == "offLed") {
      digitalWrite(ledRed, LOW);
    } else {
      Serial.println("Problème messageTemp pour LedRed");
    }
  }

  if (String(topic) == "esp32/maison/buzzerHz") {
    freq_tone = (unsigned int)messageTemp.toInt();
    tone(buzzer, freq_tone, time_tone);
  }

  if (String(topic) == "esp32/maison/buzzerTime") {
    time_tone = (unsigned long)messageTemp.toInt();
    tone(buzzer, freq_tone, time_tone);
  }

  if (String(topic) == "esp32/maison/ledStickRed") {
    red = messageTemp.toInt();
    colorWipe(strip.Color(green, red, blue), 100);
  }

  if (String(topic) == "esp32/maison/ledStickGreen") {
    green = messageTemp.toInt();
    colorWipe(strip.Color(green, red, blue), 100);
  }

  if (String(topic) == "esp32/maison/ledStickBlue") {
    blue = messageTemp.toInt();
    colorWipe(strip.Color(green, red, blue), 100);
  }

  if (String(topic) == "esp32/maison/ledStickRainbow") {
    rainbow(10);
  }

  if (String(topic) == "esp32/maison/ecran2") {
    display.setTextSize(messageTemp.toInt());
    affichageScan(2, lastMessageOLED);
    Serial.println("Changement taille texte");
  }
  if (String(topic) == "esp32/maison/ecran") {
    if (messageTemp == "EraseAll") {
      affichageScan(0, "a");
      lastMessageOLED = "Scan badge";
    } else {
      affichageScan(2, messageTemp);
      lastMessageOLED = messageTemp;
    }
  }
}



//La fonction "reconnect()" est utilisée pour garantir la connexion MQTT entre l'ESP32 et le broker MQTT.
//Elle est appelée dans la boucle principale et elle se répète jusqu'à ce que la connexion soit établie avec succès.
void reconnect() {
  while (!clientTHUNE.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (clientTHUNE.connect("espClientTHUNE")) {
      Serial.println("connected");
      // Subscribe
      clientTHUNE.subscribe("esp32/maison/ledRed");
      clientTHUNE.subscribe("esp32/maison/buzzerHz");
      clientTHUNE.subscribe("esp32/maison/buzzerTime");
      clientTHUNE.subscribe("esp32/maison/door");
      clientTHUNE.subscribe("esp32/maison/ledStickRed");
      clientTHUNE.subscribe("esp32/maison/ledStickBlue");
      clientTHUNE.subscribe("esp32/maison/ledStickGreen");
      clientTHUNE.subscribe("esp32/maison/ledStickRainbow");
      clientTHUNE.subscribe("esp32/maison/ecran");
      clientTHUNE.subscribe("esp32/maison/ecran2");
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientTHUNE.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


int lastLedState = 0;


void checkingData() {
  if (!clientTHUNE.connected()) {
    reconnect();
  }
  clientTHUNE.loop();
  long now = millis();
  if (now - lastMsg > 500) {
    lastMsg = now;

    int ledRedState = digitalRead(ledRed);

    if (ledRedState != lastLedState) {
      if (ledRedState == LOW) {
        clientTHUNE.publish("esp32/maison/ledRedDisplay", "off");
        Serial.println("Led OFF");
      } else {
        clientTHUNE.publish("esp32/maison/ledRedDisplay", "on");
        Serial.println("Led ON");
      }
      lastLedState = ledRedState;
    }

    if (lastStateDoor == "open") {
      clientTHUNE.publish("esp32/maison/doorState", "Ouverte");
      lastStateDoor = "null";
    } else if (lastStateDoor == "close") {
      clientTHUNE.publish("esp32/maison/doorState", "Fermée");
      lastStateDoor = "null";
    }
  }
}

//Pour envoyer les données dans la base SQL

void sendData(int identifiant) {

  if (identifiant == 1) {
    Serial.println("Données envoyés OPEN");
    clientTHUNE.publish("esp32/maison/ouverture", "1");
  } else if (identifiant == 2) {
    Serial.println("Données envoyés OPEN");
    clientTHUNE.publish("esp32/maison/ouverture", "2");
  } else if (identifiant == 0) {
    clientTHUNE.publish("esp32/maison/erreur", "null");
    Serial.println("Données envoyés ERREUR");
  } else {
    Serial.println("Problème dans l'identifiant entré");
  }
}
