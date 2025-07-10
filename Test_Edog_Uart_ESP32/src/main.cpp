#include <Arduino.h>
#include <VescUart.h>

VescUart MyVescUart;

/** UART matériel utilisé sur l'ESP32 */
HardwareSerial VESCSerial(2); // UART2

void maxAngleDiffTest();

// Définir les broches RX/TX utilisées pour la communication UART avec le VESC
#define RXD2 16
#define TXD2 17

void homing();

void setup() {
  Serial.begin(115200); 
  VESCSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  MyVescUart.setSerialPort(&VESCSerial);
  
  delay(6000);
  Serial.println("Début du homing");
  homing();  // Calibre la position home
  delay(1000);
}

void loop() {
  
  Serial.println("Début du homing");
  homing();  // Calibre la position home
  /*
  if (!MyVescUart.getVescValues()) {
    for(int i = 0 ; i < 45 ; i++){
      MyVescUart.setPos(i); // Positionne le moteur à i degrés
      delay(5);
    }

    delay(1000); 


    for(int i = 0 ; i < 90 ; i++){
      MyVescUart.setPos(i,3); 
      delay(5);
    }

    delay(1000);


    for(int i = 90 ; i > 0 ; i--){
      MyVescUart.setPos(i,3); 
      delay(5);
    }

      delay(1000); 

    for(int i = 45 ; i > 0 ; i--){
      MyVescUart.setPos(i); 
      delay(5);
    }

    delay(1000);
  }else {
    Serial.println("Errruhgerbgeriueur de lecture des valeurs VESC pour homing.");
  }*/
  delay(2500);
}

// Homing: on considère la position actuelle comme 0
void homing() {
  if (MyVescUart.getVescValues()) {
    float current_pos = MyVescUart.data.pidPos;  // position actuelle en tours
    Serial.print("Position actuelle VESC : ");
    Serial.println(current_pos, 6);
    delay(5);

    Serial.println(current_pos, 6);
    Serial.print("----------------------------------------");
    
    MyVescUart.setPidPosOffset(0);  // envoi la commande
    MyVescUart.setPidPosOffset(0, 3);

    current_pos = MyVescUart.data.pidPos;  // position actuelle en tours
    Serial.print("Position 2 actuelle VESC : ");
    for(int i = 0 ; i < 5 ; i++){
      Serial.println(current_pos, 6);
      delay(5);
    }
    Serial.println(current_pos, 6);

    delay(1000);

    MyVescUart.setPidPosOffset(current_pos);  // envoi la commande
    MyVescUart.setPidPosOffset(current_pos, 3);

    delay(1000);  // temps pour que le VESC applique l'offset

    Serial.println("Commande offset PID position envoyée (homing OK).");
    delay(1000);
  } else {
    Serial.println("Erreur de lecture des valeurs VESC pour homing.");
  }
}