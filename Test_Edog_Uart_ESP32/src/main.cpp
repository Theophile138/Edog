#include <Arduino.h>
#include <VescUart.h>

VescUart MyVescUart;
HardwareSerial VESCSerial(2);

#define RXD2 16
#define TXD2 17

void homing();
void testMove();

void setup() {
  Serial.begin(115200);
  VESCSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  MyVescUart.setSerialPort(&VESCSerial);

  delay(2000);
  Serial.println("Début du homing");
  homing();  // Calibre la position home
  delay(1000);

  Serial.println("Déplacement test");
  testMove();
}

void loop() {
  testMove();
}

// Homing: on considère la position actuelle comme 0
void homing() {
  if (MyVescUart.getVescValues()) {
    float current_pos = MyVescUart.data.pidPos;  // position actuelle en tours
    Serial.print("Position actuelle VESC (tours): ");
    Serial.println(current_pos, 6);

    MyVescUart.setPidPosOffset(current_pos, 3);  // envoi la commande

    delay(100);  // temps pour que le VESC applique l'offset

    Serial.println("Commande offset PID position envoyée (homing OK).");
  } else {
    Serial.println("Erreur de lecture des valeurs VESC pour homing.");
  }
}

// Test: déplacement à 90° (0.25 tours) puis retour à 0
void testMove() {
  Serial.println("Déplacement vers 90 degrés (0.25 tours)");
  MyVescUart.setPos(25, 3);  // 90° en tours
  delay(3000);

  Serial.println("Retour à la position home (0 tours)");
  MyVescUart.setPos(0, 3);
  delay(3000);

  Serial.println("Fin du test de déplacement.");
}