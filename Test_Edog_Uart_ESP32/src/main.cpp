#include <Arduino.h>
#include <VescUart.h>
#include "moteur.h"

VescUart MyVescUart;
VescUart MyVescUart2;

Moteur Moteur1(&MyVescUart, 0, 20, 1 , false); // Création d'une instance de la classe Moteur
Moteur Moteur2(&MyVescUart2, 0, 10, 1 , false); // Création d'une instance de la classe Moteur

// ------------------- ESP32 -------------------

/** UART matériel utilisé sur l'ESP32 */
//HardwareSerial VESCSerial(2); // UART2

// Définir les broches RX/TX utilisées pour la communication UART avec le VESC
//#define RXD2 16
//#define TXD2 17

// ---------------------------------------------

#define HWSERIAL Serial2
#define HWSERIAL2 Serial5

String inputString = "";   
bool inputComplete = false;  

int marcheStep = 0;
bool activeMarche = false;

void parseCommand(String command);
void marche();
void marcheRefresh();
void handleButtonInterrupt();

#define BUTTON_PIN 33
volatile bool buttonPressed = false;

void setup() {
  Serial.begin(115200); 
  
  //Pour esp32 
  //VESCSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  //MyVescUart.setSerialPort(&VESCSerial);

  HWSERIAL.begin(115200);
  HWSERIAL2.begin(115200);

  MyVescUart.setSerialPort(&HWSERIAL);
  MyVescUart2.setSerialPort(&HWSERIAL2);
  
  
  inputString.reserve(50); 

  delay(1000);

  Serial.println(Moteur1.begin()); // Initialisation du moteur 1
  Serial.println(Moteur2.begin()); // Initialisation du moteur 2

  Moteur1.SoftwareOffset(0.0f); // Position initiale du moteur 1
  Moteur2.SoftwareOffset(0.0f); // Position initiale du

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Le bouton connecté à GND
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

}

void loop() {
  //Moteur1.Refresh();
  //Moteur2.Refresh();
  
  Moteur1.Refresh_Values(); // Met à jour les valeurs du moteur 1
  Moteur2.Refresh_Values(); // Met à jour les valeurs du moteur 2

  marcheRefresh();

  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      inputComplete = true;
    } else {
      inputString += inChar;
    }
  }

  if (inputComplete) {
    parseCommand(inputString);  
    inputString = "";           
    inputComplete = false;
  }

    if (buttonPressed) {
    buttonPressed = false; // Reset le flag
      activeMarche = false;
    Moteur1.stop();         // Appel sûr ici
    Moteur2.stop();
    Serial.println("button stop appuyer");

    delay(500); // Attendre un peu pour éviter les rebonds du bouton
  }
}

bool isNumber(String str) {
  str.trim();
  if (str.length() == 0) return false;
  bool decimalPoint = false;
  for (unsigned int i = 0; i < str.length(); i++) {
    if (isDigit(str.charAt(i))) continue;
    if (str.charAt(i) == '.' && !decimalPoint) {
      decimalPoint = true;
      continue;
    }
    return false;
  }
  return true;
}

void marcheRefresh() {
  if (activeMarche) {
    marche();
  }
}

void marche(){

  if(Moteur1.finish() && Moteur2.finish()) {
    if (marcheStep == 0) {
      Serial.println("Marche étape 1 : Moteur1 à 90° et Moteur2 à 45°");
      Moteur1.setTargetPos(110.0f); // Position du moteur 1
      Moteur2.setTargetPos(180.0f); // Position du moteur 2
      marcheStep = 1;
    } else if (marcheStep == 1) {
      Serial.println("Marche étape 2 : Moteur1 à 0° et Moteur2 à 0°");
      Moteur1.setTargetPos(0.0f); // Position du moteur 1
      Moteur2.setTargetPos(0.0f); // Position du moteur 2
      marcheStep = 2;
    } else {
      Serial.println("Marche terminée, réinitialisation des moteurs.");
      marcheStep = 0;
    }
  }
}


void parseCommand(String command) {
  command.trim();
  command.toLowerCase();

  // Découpe la commande en deux mots (cmd + cible)
  int spaceIndex = command.indexOf(' ');
  if (spaceIndex == -1) {
    // Ajout pour détecter "marche" sans argument
    if (command == "marche") {
      activeMarche = true;
      return;
    } else if (command == "arret") {
      activeMarche = false;
      marcheStep = 0;
      Moteur1.setTargetPos(0.0f); // Position du moteur 1
      Moteur2.setTargetPos(0.0f); // Position du moteur 2
      Serial.println("Marche arrêtée.");
      return;
    } else if (command == "stop"){
      Moteur1.stop();
      Moteur2.stop();
    }
    Serial.println("Commande invalide !");
    return;
  }

  String action = command.substring(0, spaceIndex);
  String target = command.substring(spaceIndex + 1);

  bool commandValide = true;

  // --- Commandes à argument numérique ---
  if (isNumber(target)) {
    float value = target.toFloat();

    if (action == "moteur1") {
      Moteur1.setTargetPos(value);
      Serial.println("Moteur1 → pos = " + String(value));
    } else if (action == "moteur2") {
      Moteur2.setTargetPos(value);
      Serial.println("Moteur2 → pos = " + String(value));
    } else {
      commandValide = false;
    }

  // --- Commandes sans nombre ---
  } else {
    if (target == "moteur1") {
      if (action == "homing") {
        Moteur1.SoftwareOffset(0.0f);
        Serial.println("Homing Moteur1");
        Serial.println(Moteur1.getCurrentPosition());
      } else if (action == "check") {
        if (Moteur1.isConnected()) {
          Serial.println("Moteur1 connecté");
        } else {
          Serial.println("Moteur1 non connecté");
        }
      } else {
        commandValide = false;
      }

    } else if (target == "moteur2") {
      if (action == "homing") {
        Moteur2.SoftwareOffset(0.0f);
        Serial.println("Homing Moteur2");
        Serial.println(Moteur2.getCurrentPosition());
      } else if (action == "check") {
        if (Moteur2.isConnected()) {
          Serial.println("Moteur2 connecté");
        } else {
          Serial.println("Moteur2 non connecté");
        }
      } else {
        commandValide = false;
      }

    } else {
      commandValide = false;
    }
  }

  if (!commandValide) {
    Serial.println("Commande inconnue : " + command);
  }
}

void handleButtonInterrupt() {
  buttonPressed = true;
}
