#include <Arduino.h>
#include <VescUart.h>
#include "moteur.h"
#include "command_interpreter.h"

#include "InverseKinematics.h"

VescUart Vesc_Port_Uart_2;
VescUart Vesc_Port_Uart_5;

Moteur Moteur2(&Vesc_Port_Uart_2, 0, 2, 0.25 , false);
Moteur Moteur1(&Vesc_Port_Uart_5, 0, 2, 0.117 , false);

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

int jump_var = 0;
bool activeJump = false;
float jump_height = 0; 
float jump_maxAngle = 0.20f;

bool constantJump = false;

bool activeKeepAlive = false;

bool activeCircleMarche = false;
float circleMarche_a = 0;

bool activeCarreMarche = false;
float carreMarche_a = 0;

void parseCommand(String command);
void marche();
void marcheRefresh();
void handleButtonInterrupt();

void jump();
void jumpRefresh();

DataPoint curve_circle(float a);
void circleMarche();
void circleMarcheRefresh();

DataPoint curve_carre(float a);
void carreMarche();
void carreMarcheRefresh();

void Comm_Marche(String arg);
void Comm_Arret(String arg);
void Comm_Stop(String arg);
void Comm_Position_Moteur1(String arg);
void Comm_Position_Moteur2(String arg);
void Comm_Homming(String arg);
void Comm_Check(String arg);
void Comm_Get_Position(String arg);
void Comm_Position_Inverse_Kinematic(String arg);
void Comm_Jump(String arg);
void Comm_Constant_Jump(String arg);
void Comm_AutoStart(String arg);
void Comm_Start(String arg);
void Comm_KeepAlive(String arg);
void Comm_CircleMarche(String arg);
void Comm_CarreMarche(String arg);


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
  Serial1.begin(115200);
  Serial5.begin(115200);
  
  Vesc_Port_Uart_2.setSerialPort(&Serial5);
  Vesc_Port_Uart_5.setSerialPort(&Serial1);
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
  MyInterpreter.addCommand("get_position",Comm_Get_Position);
  MyInterpreter.addCommand("ik", Comm_Position_Inverse_Kinematic);
  MyInterpreter.addCommand("jump", Comm_Jump);
  MyInterpreter.addCommand("autoStart", Comm_AutoStart);
  MyInterpreter.addCommand("start", Comm_Start);
  MyInterpreter.addCommand("keepAlive", Comm_KeepAlive);
  MyInterpreter.addCommand("constantJump", Comm_Constant_Jump);
  MyInterpreter.addCommand("circleMarche", Comm_CircleMarche);
  MyInterpreter.addCommand("carreMarche", Comm_CarreMarche);
  // ---------------------------------------------
}

void loop() {
  //Moteur1.Refresh();
  //Moteur2.Refresh();
  
  MyInterpreter.handle();

  Moteur1.Refresh_Values(); 
  Moteur2.Refresh_Values(); 

  marcheRefresh();
  jumpRefresh();
  circleMarcheRefresh();
  carreMarcheRefresh();

  if (buttonPressed) {
    buttonPressed = false;
    activeMarche = false;
    Moteur1.stop();        
    Moteur2.stop();
    Serial.println("button stop appuyer");
  }

  if (activeKeepAlive) {
    Moteur1.keepAlive();
    Moteur2.keepAlive();
    Serial.println("Send keep alive");
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
      Moteur1.setTargetPos(80.0f); // Position du moteur 1
      Moteur2.setTargetPos(-120.0f); // Position du moteur 2
      marcheStep = 1;
    } else if (marcheStep == 1) {
      Serial.println("Marche étape 2 : Moteur1 à 0° et Moteur2 à 0°");
      Moteur1.setTargetPos(45.0f); // Position du moteur 1
      Moteur2.setTargetPos(-45.0f); // Position du moteur 2
      marcheStep = 2;
    } else {  
      Serial.println("Marche terminée, réinitialisation des moteurs.");
      marcheStep = 0;
    }
  }
}

void circleMarcheRefresh() {
  if (activeCircleMarche) {
    circleMarche();
  }
}

void circleMarche(){
  if(Moteur1.finish() && Moteur2.finish()) {

    Serial.println("Circle marche : ");

    float d1 = 25.5f;   // longueur premier segment
    float d2 = 18.5f;   // longueur second segment
    float theta_max = 130.0f; // limite du genou en degrés

    AngleMoteur actualAngles = {Moteur1.getCurrentPosition(), Moteur2.getCurrentPosition()};

    DataFunction Result = interpolate_data_point(curve_circle, circleMarche_a, d1, d2, theta_max, actualAngles);

    Moteur1.setMaxAngleDiff(Result.maxAngle_phi);
    Moteur2.setMaxAngleDiff(Result.maxAngle_theta);

    // Applique les positions aux moteurs
    Moteur1.setTargetPos(Result.phi);
    Moteur2.setTargetPos(Result.theta);

    circleMarche_a += 0.1f;
    if (circleMarche_a > 80.0f) {
      circleMarche_a = 0.0f;
    }

    Serial.println(circleMarche_a);
  }
}

DataPoint curve_circle(float a) {
  DataPoint result;
  result.maxAngle = 0.25f; // Vitesse maximale des moteurs

  if ((a >= 0.0f)&&(a<= 40.0f)){
    result.x = a - 20.0f;
    float gamma= 25.0f;
    float alpha = (40.0f - gamma )/400.0f;
    result.y = alpha * result.x * result.x +gamma;
  } else if ((a > 40.0f)&&(a<= 80.0f)){
    result.x = 60.0f - a;
    result.y = 40.0f;
  }
  
  return result;
}

void carreMarcheRefresh() {
  if (activeCarreMarche) {
    carreMarche();
  }
}

void carreMarche(){
  if(Moteur1.finish() && Moteur2.finish()) {

    Serial.println("Carre marche : ");

    float d1 = 25.5f;   // longueur premier segment
    float d2 = 18.5f;   // longueur second segment
    float theta_max = 130.0f; // limite du genou en degrés

    AngleMoteur actualAngles = {Moteur1.getCurrentPosition(), Moteur2.getCurrentPosition()};

    DataFunction Result = interpolate_data_point(curve_carre, carreMarche_a, d1, d2, theta_max, actualAngles);

    Moteur1.setMaxAngleDiff(Result.maxAngle_phi);
    Moteur2.setMaxAngleDiff(Result.maxAngle_theta);

    // Applique les positions aux moteurs
    Moteur1.setTargetPos(Result.phi);
    Moteur2.setTargetPos(Result.theta);

    carreMarche_a += 0.3f;
    if (carreMarche_a > 100.0f) {
      carreMarche_a = 0.0f;
    }
  }
}

DataPoint curve_carre(float a) {
  DataPoint result;
  result.maxAngle = 0.25f; // Vitesse maximale des moteurs

  float dx = 0.0f;
  float dy = 20.0f;
  float L = 30.0f;
  float l = 20.0f;

  if ((a >= 0.0f)&&(a<= l)){
    result.x = L/2 + dx;
    result.y = l + dy - a ;
  } else if ((a > l)&&(a<= L+l)){
    result.x = dx + l + L/2 - a;
    result.y = dy;
  } else if ((a > L+l)&&(a<= L+l+l)){
    result.x = dx - L/2;
    result.y = dy + a - (L + l);
  } else if ((a > L+l+l)&&(a<= 2*L+2*l)){
    result.x = a + dx - (3/2.0f)*L - 2*l;
    result.y = dy + l;
  }
  
  return result;
}

void jumpRefresh() {
  if (activeJump) {
    jump();
  }
}

void jump() {
  if(Moteur1.finish() && Moteur2.finish()) {

    if(jump_var == 0){      
      float x = 0.0f;   // position x de la cible
      float y = 20.0f;  // position y de la cible

      Serial.println("Jump étape 1 : ik "+String(x)+" "+String(y));

      float d1 = 25.5f;   // longueur premier segment
      float d2 = 18.5f;   // longueur second segment
      float theta_max = 130.0f; // limite du genou en degrés

      // Calcul IK
      AngleMoteur angles = inverse_kinematics_2d_flexible(x, y, d1, d2, theta_max);

      AngleMoteur currentAngles = {Moteur1.getCurrentPosition(), Moteur2.getCurrentPosition()};
    
      MaxAngleMoteur maxAnglesNow = compute_max_angle(currentAngles, angles, 0.25f);

      Moteur1.setMaxAngleDiff(maxAnglesNow.maxAngle_phi);
      Moteur2.setMaxAngleDiff(maxAnglesNow.maxAngle_theta);

      // Applique les positions aux moteurs
      Moteur1.setTargetPos(angles.phi);
      Moteur2.setTargetPos(angles.theta);

      jump_var = 1;
    }else if (jump_var == 1){      
      delay(300);
      float x = 0.0f;   // position x de la cible
      float y = 20.0f + jump_height;  // position y de la cible

      Serial.println("Jump étape 2 : ik "+String(x)+" "+String(y));

      float d1 = 25.5f;   // longueur premier segment
      float d2 = 18.5f;   // longueur second segment
      float theta_max = 130.0f; // limite du genou en degrés

      // Calcul IK
      AngleMoteur angles = inverse_kinematics_2d_flexible(x, y, d1, d2, theta_max);

      AngleMoteur currentAngles = {Moteur1.getCurrentPosition(), Moteur2.getCurrentPosition()};
    
      MaxAngleMoteur maxAnglesNow = compute_max_angle(currentAngles, angles, jump_maxAngle);

      Moteur1.setMaxAngleDiff(maxAnglesNow.maxAngle_phi);
      Moteur2.setMaxAngleDiff(maxAnglesNow.maxAngle_theta);

      // Applique les positions aux moteurs
      Moteur1.setTargetPos(angles.phi);
      Moteur2.setTargetPos(angles.theta);

      jump_var = 2;
    }else if (jump_var == 2){      
      delay(1000);
      float x = 0.0f;   // position x de la cible
      float y = 20.0f;  // position y de la cible

      Serial.println("Jump étape 3 : ik "+String(x)+" "+String(y));

      float d1 = 25.5f;   // longueur premier segment
      float d2 = 18.5f;   // longueur second segment
      float theta_max = 130.0f; // limite du genou en degrés

      // Calcul IK
      AngleMoteur angles = inverse_kinematics_2d_flexible(x, y, d1, d2, theta_max);

      AngleMoteur currentAngles = {Moteur1.getCurrentPosition(), Moteur2.getCurrentPosition()};
    
      MaxAngleMoteur maxAnglesNow = compute_max_angle(currentAngles, angles, 0.25f);

      Moteur1.setMaxAngleDiff(maxAnglesNow.maxAngle_phi);
      Moteur2.setMaxAngleDiff(maxAnglesNow.maxAngle_theta);

      // Applique les positions aux moteurs
      Moteur1.setTargetPos(angles.phi);
      Moteur2.setTargetPos(angles.theta);

      jump_var = 0;
      
      if(constantJump == false){
        activeJump = false;
      }
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
  if (activeMarche) {
    activeMarche = false;
    marcheStep = 0;
    Moteur1.setTargetPos(0.0f); // Position du moteur 1
    Moteur2.setTargetPos(0.0f); // Position du moteur 2
    Serial.println("Marche arrêtée.");
  }
  if (activeJump) {
    activeJump = false;
    constantJump = false;
    jump_var = 0;

    float x = 0.0f;   // position x de la cible
    float y = 20.0f;  // position y de la cible

    // Paramètres géométriques de ta patte (longueurs des segments en cm par ex.)
    float d1 = 25.5f;   // longueur premier segment
    float d2 = 18.5f;   // longueur second segment
    float theta_max = 130.0f; // limite du genou en degrés

    // Calcul IK
    AngleMoteur angles = inverse_kinematics_2d_flexible(x, y, d1, d2, theta_max);

    AngleMoteur currentAngles = {Moteur1.getCurrentPosition(), Moteur2.getCurrentPosition()};
    
    MaxAngleMoteur maxAnglesNow = compute_max_angle(currentAngles, angles, 0.25f);

    Moteur1.setMaxAngleDiff(maxAnglesNow.maxAngle_phi);
    Moteur2.setMaxAngleDiff(maxAnglesNow.maxAngle_theta);

    // Applique les positions aux moteurs
    Moteur1.setTargetPos(angles.phi);
    Moteur2.setTargetPos(angles.theta);

    Serial.println("Jump arrêtée.");
  }
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

void Comm_Get_Position(String arg){

  const int TotalArgs = 1;
  String args[TotalArgs];

  int numArg = Commande_interpreter::splitArgs(arg,args,TotalArgs);

  if (numArg == 1) { 
    if (args[0] == "moteur1"){
        Serial.print("Position Moteur1 : ");
        Serial.println(Moteur1.getCurrentPosition());
        Serial.print("Reel Position Moteur1 : ");
        Serial.println(Moteur1.getRealPosition());
    }else if (args[0] == "moteur2")
    {
        Serial.print("Position Moteur2 : ");
        Serial.println(Moteur2.getCurrentPosition());
        Serial.print("Reel Position Moteur2 : ");
        Serial.println(Moteur2.getRealPosition());
    }else{
      Serial.println("Erreur : mauvais argument");
    }
    
  } else {
    Serial.println("Erreur : argument manquant");
  }
}

void Comm_Position_Inverse_Kinematic(String arg) {
  const int TotalArgs = 2;
  String args[TotalArgs];

  int numArg = Commande_interpreter::splitArgs(arg, args, TotalArgs);

  if (numArg == 2) {
    float x = args[0].toFloat();
    float y = args[1].toFloat();

    // Paramètres géométriques de ta patte (longueurs des segments en cm par ex.)
    float d1 = 25.5f;   // longueur premier segment
    float d2 = 18.5f;   // longueur second segment
    float theta_max = 130.0f; // limite du genou en degrés

    // Calcul IK
    AngleMoteur angles = inverse_kinematics_2d_flexible(x, y, d1, d2, theta_max);

    AngleMoteur currentAngles = {Moteur1.getCurrentPosition(), Moteur2.getCurrentPosition()};
    
    MaxAngleMoteur maxAnglesNow = compute_max_angle(currentAngles, angles, 0.25f);

    Moteur1.setMaxAngleDiff(maxAnglesNow.maxAngle_phi);
    Moteur2.setMaxAngleDiff(maxAnglesNow.maxAngle_theta);

    // Applique les positions aux moteurs
    Moteur1.setTargetPos(angles.phi);
    Moteur2.setTargetPos(angles.theta);

    Serial.print("IK → X=");
    Serial.print(x);
    Serial.print(" Y=");
    Serial.print(y);
    Serial.print(" => Phi=");
    Serial.print(angles.phi);
    Serial.print("°, Theta=");
    Serial.println(angles.theta);
  } else {
    Serial.println("Erreur : il faut fournir X et Y → exemple : ik 10 5");
  }
}

void Comm_Jump(String arg) {
  const int TotalArgs = 2;
  String args[TotalArgs];

  int numArg = Commande_interpreter::splitArgs(arg,args,TotalArgs);

  if (numArg == 2) { 
    jump_height = args[0].toFloat();  
    jump_maxAngle = args[1].toFloat(); 

    Serial.println("JUMP !");
    activeJump = true;
    jump_var = 0;
  } else {
    Serial.println("Erreur : argument manquant pour le jump");
  }
  

}

void Comm_AutoStart(String arg) {
  Moteur1.SoftwareOffset(0.0f); 
  Moteur2.SoftwareOffset(0.0f);

  Serial.println("set offset");
  delay(100);

  Moteur1.setTargetPos(0.0f); // Position du moteur 1
  Moteur2.setTargetPos(0.0f); // Position du moteur 2
  
  while(!Moteur1.finish() || !Moteur2.finish()){
    Moteur1.Refresh_Values(); 
    Moteur2.Refresh_Values(); 
  }

  Serial.println("set pos 0");
  delay(1000);

  Moteur1.setTargetPos(-95.0f); // Position du moteur 1
  
  while(!Moteur1.finish() || !Moteur2.finish()){
    Moteur1.Refresh_Values(); 
    Moteur2.Refresh_Values(); 
  }

  Serial.println("set pos -90");
  delay(500);

  Moteur1.SoftwareOffset(0.0f);

  delay(500);
  
  Moteur1.setTargetPos(0.0f); // Position du moteur 1

  while(!Moteur1.finish() || !Moteur2.finish()){
    Moteur1.Refresh_Values(); 
    Moteur2.Refresh_Values(); 
  }
    
}

void Comm_Start(String arg) {
  Moteur1.SoftwareOffset(0.0f); 
  Moteur2.SoftwareOffset(0.0f);

  Serial.println("set offset");
  delay(100);

  Moteur1.setTargetPos(0.0f); // Position du moteur 1
  Moteur2.setTargetPos(0.0f); // Position du moteur 2
  
  while(!Moteur1.finish() || !Moteur2.finish()){
    Moteur1.Refresh_Values(); 
    Moteur2.Refresh_Values(); 
  }

  Serial.println("set pos 0");
  delay(1000);
    
}

void Comm_KeepAlive(String arg) {
  activeKeepAlive = !activeKeepAlive;
}

void Comm_Constant_Jump(String arg) {
  const int TotalArgs = 2;
  String args[TotalArgs];

  int numArg = Commande_interpreter::splitArgs(arg,args,TotalArgs);

  if (numArg == 2) { 
    jump_height = args[0].toFloat();  
    jump_maxAngle = args[1].toFloat(); 

    Serial.println("JUMP !");
    activeJump = true;
    jump_var = 0;
  } else {
    Serial.println("Erreur : argument manquant pour le jump");
  }
  
  constantJump = true;
}

void Comm_CircleMarche(String arg) {
  activeCircleMarche = !activeCircleMarche;

  if (activeCircleMarche) {
    Serial.println("Circle marche activée");
  } else {
    Serial.println("Circle marche désactivée");
  }
}

void Comm_CarreMarche(String arg) {
  activeCarreMarche = !activeCarreMarche;

  if (activeCarreMarche) {
    Serial.println("Carre marche activée");
  } else {
    Serial.println("Carre marche désactivée");
  }
}