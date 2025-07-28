#include <Arduino.h>
#include <VescUart.h>
#include "moteur.h"
#include "command_interpreter.h"

VescUart Vesc_Port_Uart_2;
VescUart Vesc_Port_Uart_5;

Moteur Moteur1(&Vesc_Port_Uart_2, 0, 20, 1 , false);
Moteur Moteur2(&Vesc_Port_Uart_5, 0, 10, 1 , false);

Commande_interpreter MyInterpreter;

// ------------------- ESP32 -------------------

/** UART matériel utilisé sur l'ESP32 */
//HardwareSerial VESCSerial(2); // UART2

// Définir les broches RX/TX utilisées pour la communication UART avec le VESC
//#define RXD2 16
//#define TXD2 17

// ---------------------------------------------
int marcheStep = 0;
bool activeMarche = false;

void parseCommand(String command);
void marche();
void marcheRefresh();
void handleButtonInterrupt();

void Comm_Marche(String arg);
void Comm_Arret(String arg);
void Comm_Stop(String arg);
void Comm_Position_Moteur1(String arg);
void Comm_Position_Moteur2(String arg);
void Comm_Homming(String arg);
void Comm_Check(String arg);

#define BUTTON_PIN 33
volatile bool buttonPressed = false;

void setup() {
   
  Serial.begin(115200);  
  MyInterpreter.begin(&Serial);

  // ------------------- ESP32 -------------------
  //VESCSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  //MyVescUart.setSerialPort(&VESCSerial);
  // ---------------------------------------------

  // ------------------- Teensy ------------------
  Serial2.begin(115200);
  Serial5.begin(115200);
  
  Vesc_Port_Uart_2.setSerialPort(&Serial5);
  Vesc_Port_Uart_5.setSerialPort(&Serial2);
  // ---------------------------------------------

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

  // ------------- Adding commande ---------------
  MyInterpreter.addCommand("marche",Comm_Marche);
  MyInterpreter.addCommand("arret",Comm_Arret);
  MyInterpreter.addCommand("stop",Comm_Stop);
  MyInterpreter.addCommand("moteur1",Comm_Position_Moteur1);
  MyInterpreter.addCommand("moteur2",Comm_Position_Moteur2);
  MyInterpreter.addCommand("homing",Comm_Homming);
  MyInterpreter.addCommand("check",Comm_Check);
  // ---------------------------------------------
}

void loop() {
  //Moteur1.Refresh();
  //Moteur2.Refresh();
  
  MyInterpreter.handle();

  Moteur1.Refresh_Values(); 
  Moteur2.Refresh_Values(); 

  marcheRefresh();

  if (buttonPressed) {
    buttonPressed = false;
    activeMarche = false;
    Moteur1.stop();        
    Moteur2.stop();
    Serial.println("button stop appuyer");
  }
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

void handleButtonInterrupt() {
  buttonPressed = true;
}

// ------------- Function for command ---------------

void Comm_Marche(String arg){
  activeMarche = true;
}

void Comm_Arret(String arg){
  activeMarche = false;
  marcheStep = 0;
  Moteur1.setTargetPos(0.0f); // Position du moteur 1
  Moteur2.setTargetPos(0.0f); // Position du moteur 2
  Serial.println("Marche arrêtée.");
}

void Comm_Stop(String arg){
  const int TotalArgs = 1;
  String args[TotalArgs];

  int numArg = Commande_interpreter::splitArgs(arg,args,TotalArgs);

  if (numArg == 0){
    Moteur1.stop();
    Moteur2.stop();
  }

  if (numArg == 1){
    if (args[0] == "moteur1"){
      Moteur1.stop();
    }else if (args[0] == "moteur2"){
      Moteur2.stop();
    }else {
      MyInterpreter.println("Error arg not correct :" + args[0]);
    }
  }
}

void Comm_Position_Moteur1(String arg){

  const int TotalArgs = 1;
  String args[TotalArgs];

  int numArg = Commande_interpreter::splitArgs(arg,args,TotalArgs);

  if (numArg == 1) { 
    float value = args[0].toFloat();  
    Moteur1.setTargetPos(value);
    Serial.println("Moteur1 → pos = " + String(value));
  } else {
    Serial.println("Erreur : argument manquant pour Moteur1");
  }
}

void Comm_Position_Moteur2(String arg){

  const int TotalArgs = 1;
  String args[TotalArgs];

  int numArg = Commande_interpreter::splitArgs(arg,args,TotalArgs);

  if (numArg == 1) { 
    float value = args[0].toFloat();  
    Moteur2.setTargetPos(value);
    Serial.println("Moteur2 → pos = " + String(value));
  } else {
    Serial.println("Erreur : argument manquant pour Moteur1");
  }
}

void Comm_Homming(String arg){

  const int TotalArgs = 1;
  String args[TotalArgs];

  int numArg = Commande_interpreter::splitArgs(arg,args,TotalArgs);

  if (numArg == 1) { 
    if (args[0] == "moteur1"){
      Moteur1.SoftwareOffset(0.0f);
      Serial.println("Homing Moteur1");
      Serial.println(Moteur1.getCurrentPosition());
    }else if (args[0] == "moteur2")
    {
      Moteur2.SoftwareOffset(0.0f);
      Serial.println("Homing Moteur2");
      Serial.println(Moteur2.getCurrentPosition());
    }else{
      Serial.println("Erreur : mauvais argument");
    }
    
  } else {
    Serial.println("Erreur : argument manquant");
  }
}
void Comm_Check(String arg){

  const int TotalArgs = 1;
  String args[TotalArgs];

  int numArg = Commande_interpreter::splitArgs(arg,args,TotalArgs);

  if (numArg == 1) { 
    if (args[0] == "moteur1"){
      if (Moteur1.isConnected()) {
        Serial.println("Moteur1 connecté");
      } else {
        Serial.println("Moteur1 non connecté");
      }
    }else if (args[0] == "moteur2")
    {
        if (Moteur2.isConnected()) {
          Serial.println("Moteur2 connecté");
        } else {
          Serial.println("Moteur2 non connecté");
        }
    }else{
      Serial.println("Erreur : mauvais argument");
    }
    
  } else {
    Serial.println("Erreur : argument manquant");
  }
}