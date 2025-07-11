#ifndef MOTEUR_h
#define MOTEUR_h

#include <Arduino.h>
#include <VescUart.h>

typedef enum {
	NONE = 0,
	WRONG_NUMBER,
    NO_VESC_CONNECTION,
} ERROR;

class Moteur
{
  public:
    Moteur(VescUart* MyVescUart, uint8_t canId, int timeBetweenSteps = 5, int maxAngleDiff = 1, bool debugMode = false);
    ERROR begin();
    
    ERROR setTargetPos(float targetPos);

    // This method don't work
    void ForceSetOffset(float offset_deg);

    void SoftwareOffset(float offset_deg);

    ERROR Refresh_Values();
    ERROR Refresh();
    
    bool isConnected();
    
    float getCurrentPosition();

    bool finish();

  private:
    
    float theoricalPosToReel(float theoricalPos);

    bool _debugMode;

    bool _inPosition;

    unsigned long lastUpdateTime = 0; // Last time the motor was updated

    VescUart* _MyVescUart;  
    
    uint8_t _canId; // CAN ID of the VESC
    int _timeBetweenSteps; // Time between each step in milliseconds
    int _maxAngleDiff; // Maximum angle difference for each step

    float _targetPos; // Target position of the motor in degrees
    float _theoreticalPos; // Theoretical position of the motor in degrees

    float _softwareOffset = 0.0f; // Software offset for the motor position
};

#endif 