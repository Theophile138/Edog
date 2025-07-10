#include <Arduino.h>
#include <VescUart.h>
#include "moteur.h"

VescUart MyVescUart;

Moteur Moteur1(&MyVescUart, 0, 10, 1 , true); // Création d'une instance de la classe Moteur
Moteur Moteur2(&MyVescUart, 3, 10, 1 , true); // Création d'une instance de la classe Moteur

/** UART matériel utilisé sur l'ESP32 */
HardwareSerial VESCSerial(2); // UART2

// Définir les broches RX/TX utilisées pour la communication UART avec le VESC
#define RXD2 16
#define TXD2 17

String inputString = "";      // Stocke la ligne entrée
bool inputComplete = false;   // Drapeau de fin de ligne

void parseCommand(String command);

void setup() {
  Serial.begin(115200); 
  VESCSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  MyVescUart.setSerialPort(&VESCSerial);
  inputString.reserve(50);  // réserve un peu de mémoire

  delay(1000);

  Serial.println("Initialisation des moteurs...");

  Serial.println(Moteur1.begin()); // Initialisation du moteur 1
  Serial.println(Moteur2.begin()); // Initialisation du moteur 2
   
  Serial.println("Force homing");
  
  //Moteur1.ForceSetOffset(0.0f); // Position initiale du moteur 1
  //Moteur2.ForceSetOffset(0.0f); // Position initiale

  Moteur1.SoftwareOffset(0.0f); // Position initiale du moteur 1
  Moteur2.SoftwareOffset(0.0f); // Position initiale du

  //Moteur1.setTargetPos(90.0f); // Position cible du moteur 1
  //Moteur2.setTargetPos(90.0f); // Position cible du moteur
}

void loop() {
  // Lecture manuelle du port série
  
  Moteur1.Refresh();
  Moteur2.Refresh();
  
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      inputComplete = true;
    } else {
      inputString += inChar;
    }
  }

  // Si une ligne complète est reçue
  if (inputComplete) {
    parseCommand(inputString);  // interprétation de la commande
    inputString = "";           // reset
    inputComplete = false;
  }
}

// 📦 Fonction pour analyser la commande
void parseCommand(String command) {
  command.trim(); // supprime les espaces / \n
  command.toLowerCase();

  // Exemple de commande : "moteur1 90"
  int spaceIndex = command.indexOf(' ');
  if (spaceIndex == -1) return; // pas d'espace trouvé → commande invalide

  String moteur = command.substring(0, spaceIndex);
  String angleStr = command.substring(spaceIndex + 1);
  float angle = angleStr.toFloat();

  if (moteur == "moteur1") {
    Moteur1.setTargetPos(angle);
    Serial.println("Commande envoyée à Moteur1: " + String(angle));
  } else if (moteur == "moteur2") {
    Moteur2.setTargetPos(angle);
    Serial.println("Commande envoyée à Moteur2: " + String(angle));
  } else {
    Serial.println("Moteur inconnu : " + moteur);
  }
}