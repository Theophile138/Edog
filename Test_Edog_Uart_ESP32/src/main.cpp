#include <Arduino.h>
#include <VescUart.h>

/** Instance de VescUart */
VescUart UART;

/** UART matériel utilisé sur l'ESP32 */
HardwareSerial VESCSerial(2); // UART2

// Définir les broches RX/TX utilisées pour la communication UART avec le VESC
#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);  // Pour debug via USB

  // Important : doit correspondre à la vitesse configurée sur ton VESC
  VESCSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);

  UART.setSerialPort(&VESCSerial);

  Serial.println("ESP32 -> VESC UART ready (115200 baud).");
}

void loop() {

float pos = 0.0; // Position du moteur en degrés


  
while (true)
{
    UART.setPos(pos); // Positionne le moteur à 0.0 (en degrés)
  Serial.print("Positionnement du moteur à ");
  Serial.println(pos);

  delay(500); // Attend 3 sec
  pos = pos + 10.0;
  if (pos > 360.0) {
    pos = 0.0; // Réinitialise la position si elle dépasse 360 degrés
  }
}


/*
float num = 100.0;
while(true) {
  UART.setRPM(num); 
  //delay(3000); // Attend 3 sec
  num = num + 10.0;

    Serial.println("Arrêt du moteur...");
  if (UART.getVescValues()) {
    Serial.print("RPM: "); Serial.println(UART.data.rpm);
    Serial.print("Tension: "); Serial.println(UART.data.inpVoltage);
    Serial.print("Ah: "); Serial.println(UART.data.ampHours);
    Serial.print("Tachomètre: "); Serial.println(UART.data.tachometerAbs);
    Serial.println("-------------------------");
  } else {
    Serial.println("❌ Échec communication avec le VESC.");
  }
}


*/

  /*
  Serial.println("Arrêt du moteur...");
  if (UART.getVescValues()) {
    Serial.print("RPM: "); Serial.println(UART.data.rpm);
    Serial.print("Tension: "); Serial.println(UART.data.inpVoltage);
    Serial.print("Ah: "); Serial.println(UART.data.ampHours);
    Serial.print("Tachomètre: "); Serial.println(UART.data.tachometerAbs);
    Serial.println("-------------------------");
  } else {
    Serial.println("❌ Échec communication avec le VESC.");
  }


  // Arrête le moteur
  UART.setRPM(0);
  UART.setBrakeCurrent(1); // Applique un courant de freinage de 10A

  delay(3000);

  UART.setPos(0.0); // Positionne le moteur à 0.0 (en degrés)
  Serial.println("Positionnement du moteur à 0.0 degrés...");
  delay(3000); // Attend 3 sec
  UART.setPos(90.0); // Positionne le moteur à 90.0 (en degrés)
  Serial.println("Positionnement du moteur à 90.0 degrés...");  
  delay(3000); // Attend 3 sec
  */
}
