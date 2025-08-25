#ifndef INVERSE_KINEMATICS_H
#define INVERSE_KINEMATICS_H

#include <Arduino.h>

struct AngleMoteur {
  float phi;    // angle du premier segment (en degrés)
  float theta;  // angle du genou (en degrés)
};

struct MaxAngleMoteur {
  float maxAngle_phi;    // vitesse du premier moteur
  float maxAngle_theta;  // vitesse du second moteur
};

struct DataFunction {
    float maxAngle_phi;
    float maxAngle_theta;
    float phi;
    float theta;
};

struct DataPoint {
    float maxAngle;
    float x;
    float y;
};

/**
 * @brief Calcule les angles moteurs d'un bras 2D à 2 segments avec limite de flexion.
 * 
 * @param x position cible en X
 * @param y position cible en Y
 * @param d1 longueur du premier segment
 * @param d2 longueur du second segment
 * @param theta_max_deg angle max du genou en degrés
 * @return AngleMoteur (phi, theta) en degrés
 */
AngleMoteur inverse_kinematics_2d_flexible(float x, float y, float d1, float d2, float theta_max_deg);

MaxAngleMoteur compute_max_angle(AngleMoteur current, AngleMoteur target, float max_angle);

/**
 * @brief Interpole un DataPoint à partir d'une fonction qui retourne un Point(x,y).
 * 
 * @param func fonction qui prend un float et retourne un Point
 * @param a paramètre d'interpolation
 * @return DataPoint (maxAngle, phi, theta)
 */
DataFunction interpolate_data_point(DataPoint (*func)(float), float a,float d1, float d2, float theta_max_deg, AngleMoteur actualAngles);

#endif // INVERSE_KINEMATICS_H
