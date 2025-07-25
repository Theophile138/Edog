#include <Arduino.h>
#include <VescUart.h>
#include "moteur.h"

VescUart Vesc_Port_Uart_2;
VescUart Vesc_Port_Uart_5;

Moteur Moteur1(&Vesc_Port_Uart_2, 0, 20, 1 , false);
Moteur Moteur2(&Vesc_Port_Uart_5, 0, 10, 1 , false);

// ------------------- ESP32 -------------------

/** UART matériel utilisé sur l'ESP32 */
//HardwareSerial VESCSerial(2); // UART2

// Définir les broches RX/TX utilisées pour la communication UART avec le VESC
//#define RXD2 16
//#define TXD2 17

// ---------------------------------------------

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
    
// ------------------- ESP32 -------------------
  //VESCSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  //MyVescUart.setSerialPort(&VESCSerial);
// ---------------------------------------------

  Serial.begin(115200); 
  Serial2.begin(115200);
  Serial5.begin(115200);

  Vesc_Port_Uart_2.setSerialPort(&Serial5);
  Vesc_Port_Uart_5.setSerialPort(&Serial2);
  
  inputString.reserve(50); 

  delay(100);

  Serial.println("Initialisation des moteurs...");
  Serial.print("Etat moteur 1 : ");
  Serial.println(Moteur1.begin());
  Serial.print("Etat moteur 2 : ");
  Serial.println(Moteur2.begin());

  Moteur1.SoftwareOffset(0.0f);
  Moteur2.SoftwareOffset(0.0f); 

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Le bouton connecté à GND
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

}

void loop() {
  //Moteur1.Refresh();
  //Moteur2.Refresh();
  
  Moteur1.Refresh_Values(); 
  Moteur2.Refresh_Values(); 

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
      buttonPressed = false;
      activeMarche = false;
      Moteur1.stop();        
      Moteur2.stop();
      Serial.println("button stop appuyer");
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
      Moteur1.setTargetPos(90.0f); // Position du moteur 1
      Moteur2.setTargetPos(100.0f); // Position du moteur 2
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
      } else if (action == "debug") {

        while(true){
        if (Moteur1.isConnected()) {
          Serial.println("Moteur1 connecté");
          Serial2.println("hello");
        } else {
          Serial.println("Moteur1 non connecté");
          Serial2.println("hello");
        }
        }

      }else {
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
      }else if (action == "debug") {

        while(true){
        if (Moteur2.isConnected()) {
          Serial.println("Moteur2 connecté");
          //Serial5.println("hello");
        } else {
          Serial.println("Moteur2 non connecté");
          //Serial5.println("hello");
        }
        }

      }
       else {
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
