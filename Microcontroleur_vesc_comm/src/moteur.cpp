#include "moteur.h"

Moteur::Moteur(VescUart* MyVescUart, uint8_t canId, int timeBetweenSteps, int maxAngleDiff, bool debugMode) {
    _MyVescUart = MyVescUart;
    _canId = canId;
    _timeBetweenSteps = timeBetweenSteps;
    _maxAngleDiff = maxAngleDiff;
    _targetPos = 0.0f;
    _debugMode = debugMode;
    _inPosition =  true;
}

ERROR Moteur::begin() {
    
    ERROR result = NONE;

    if (_MyVescUart->getVescValues(_canId)) {
        _theoreticalPos = _MyVescUart->data.pidPos;
        
        if (_debugMode) {
            Serial.print("Initial Position: ");
            Serial.println(_theoreticalPos);
        }
    }else{
        _theoreticalPos = -1.0f;
        result = NO_VESC_CONNECTION;

        if(_debugMode) {
            Serial.println("Failed to get VESC values during initialization.");
        }
    }
    return result;
}

ERROR Moteur::setTargetPos(float targetPos) {
    
    ERROR errorCode = NONE;

    if (targetPos >= 0.0f && targetPos < 360.0f) {
        _targetPos = targetPos;
        _inPosition = false;
    }else{
        errorCode = WRONG_NUMBER;
    }
    
    return errorCode;
}

void Moteur::SoftwareOffset(float offset_deg) {
    
    Serial.print("Setting software offset to: ");

    if (_MyVescUart->getVescValues(_canId)) {

        _softwareOffset = _MyVescUart->data.pidPos - offset_deg; // Set the software offset
        _theoreticalPos = offset_deg; 

        if (_debugMode) {
            _MyVescUart->getVescValues(_canId); // Refresh values to get the current position
            Serial.print("Current Position after setting offset: ");
            Serial.println(_MyVescUart->data.pidPos - _softwareOffset);
        }

    } else {
        if (_debugMode) {
            Serial.println("Failed to get VESC values for software offset setting!");
        }
    }
}

// Refresh method to update motor position with the reel value from VESC (maybe slower ?)
ERROR Moteur::Refresh_Values() {
    
    unsigned long temps = millis();
    if (temps - lastUpdateTime >= _timeBetweenSteps) {
        lastUpdateTime = temps;

        if (_inPosition == false){
        
            float angleDiff = _targetPos - _theoreticalPos;

                if( angleDiff != 0) {
                
                    if (abs(angleDiff) > _maxAngleDiff) {
                        if (angleDiff > 0) {
                            _theoreticalPos = _theoreticalPos + _maxAngleDiff;
                            _MyVescUart->setPos(theoricalPosToReel(_theoreticalPos), _canId);
                            Serial.print("Moteur ID :" + String(_canId) + " Adjusting Position by Max Angle Diff: " + String(_theoreticalPos) + "\n");
                        } else {
                            _theoreticalPos = _theoreticalPos - _maxAngleDiff;
                            _MyVescUart->setPos(theoricalPosToReel(_theoreticalPos), _canId);
                            Serial.print("Moteur ID :" + String(_canId) + " Adjusting Position by Max Angle Diff: " + String(_theoreticalPos) + "\n");
                        }

                    } else {
                        _theoreticalPos = _targetPos;
                        _MyVescUart->setPos(theoricalPosToReel(_theoreticalPos), _canId);
                        Serial.print("Moteur ID :" + String(_canId) + " Adjusting Position by Max Angle Diff: " + String(_theoreticalPos) + "\n");
                    }
                }else{
                    if (_inPosition == false) {
                        _inPosition = true;
                        _MyVescUart->setPos(_targetPos + _softwareOffset, _canId); // Set the motor to the target position
                        if (_debugMode) {
                            Serial.println("Motor is in position.");
                        }
                    }else{
                        _MyVescUart->sendKeepalive(_canId);
                    }
                }
            }

    }   

    return NONE;
}

float Moteur::theoricalPosToReel(float theoricalPos) {
    float reelPos = theoricalPos + _softwareOffset;

    if (reelPos < 0.0f) {
        reelPos += 360.0f; // Ensure the position is within the range [0, 360)
    } else if (reelPos >= 360.0f) {
        reelPos -= 360.0f; // Ensure the position is within the range [0, 360)
    }

    return reelPos;
}

float Moteur::getCurrentPosition() {
    return _theoreticalPos;
}

bool Moteur::isConnected() {
    return _MyVescUart->getVescValues(_canId);
}

bool Moteur::finish() {
    return _inPosition;
}

void Moteur::stop() {

    _MyVescUart->setCurrent(0.0f, _canId); // Stop the motor by setting current to 0
    _inPosition = true; // Mark the motor as in position

    _targetPos = _theoreticalPos;

    if (_debugMode) {
        Serial.println("Motor stopped.");
    }
}