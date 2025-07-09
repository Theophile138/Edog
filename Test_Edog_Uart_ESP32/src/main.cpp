#include <Arduino.h>
#include <VescUart.h>

VescUart MyVescUart;

/** UART matériel utilisé sur l'ESP32 */
HardwareSerial VESCSerial(2); // UART2

void maxAngleDiffTest();

// Définir les broches RX/TX utilisées pour la communication UART avec le VESC
#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200); 
  VESCSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  MyVescUart.setSerialPort(&VESCSerial);
}

void loop() {

//maxAngleDiffTest();

for(int i = 0 ; i < 45 ; i++){
  MyVescUart.setPos(i); // Positionne le moteur à i degrés
  delay(5);
  //Serial.print("Positionnement du moteur à ");
  //Serial.println(i);
  
  //delay(10); // Attend 1 sec
  
  //if (MyVescUart.getVescValues()) {
  //  Serial.print("Position actuelle: ");
  //  Serial.println(MyVescUart.data.pidPos);
  //} else {
  //  Serial.println("Échec de la communication avec le VESC.");
  //}
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

}

void homing(){

}

void maxAngleDiffTest(){

  
  bool stop = false;

  float angle_recherche = 0.0;
  float angle_depart = 0.0;
  
  const int incrementation = 10.0; 

  MyVescUart.setPos(angle_recherche); 

  while(!stop){
    if (MyVescUart.getVescValues()) {

      if(MyVescUart.data.error != FAULT_CODE_NONE) {

        if( MyVescUart.data.error == FAULT_CODE_ABS_OVER_CURRENT) {
          Serial.println("Angle MAX : "+String(angle_recherche)+" Angle de départ: " + String(angle_depart));
          angle_depart += 10;
          angle_recherche = angle_depart;
          MyVescUart.setPos(angle_recherche);
        }else{
          Serial.print("Erreur VESC: ");
          Serial.println(MyVescUart.data.error);
          stop = true;
        }
      }


      Serial.print("Position actuelle: ");
      Serial.println(MyVescUart.data.pidPos);
      if (MyVescUart.data.pidPos == angle_recherche) {
        Serial.println("Position atteinte: " + String(angle_recherche));
        angle_recherche += incrementation;
        MyVescUart.setPos(angle_recherche);

      } else {
        Serial.println("Échec de la communication avec le VESC.");
        stop = true;
      }
    }
    delay(1000);
  } 
}
  

  // while (!stop){
    
  //   if (MyVescUart.getVescValues()) {

  //     if(MyVescUart.data.error != FAULT_CODE_NONE) {
  //       Serial.print("Erreur VESC: ");
  //       Serial.println(MyVescUart.data.error);

  //       if( MyVescUart.data.error == FAULT_CODE_ABS_OVER_CURRENT) {
  //         Serial.println("Moteur en surintensité");
  //         MyVescUart.setPos(angle); 
  //       }else{
  //         errorUnknown = true;
  //       }

  //     }
      
  //     if(errorUnknown == false){
  //       if (MyVescUart.data.pidPos == angle) {
          
  //       }
  //     }
  //   } else {
  //     Serial.println("Échec de la communication avec le VESC.");
  //   }
  // }


//void sensorLessHoming(uint8_t canId){
  //UART.setPos(0.0,canId);
//}

/*

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
*/

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

