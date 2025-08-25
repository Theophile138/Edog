#include "InverseKinematics.h"
#include <math.h>

AngleMoteur inverse_kinematics_2d_flexible(float x, float y, float d1, float d2, float theta_max_deg) {
  float r = hypot(x, y);

  float max_reach = d1 + d2;
  float min_reach = fabs(d1 - d2);

  // Conversion en radians pour le calcul
  float theta_max = theta_max_deg * M_PI / 180.0f;

  // Clamp de la distance
  if (r > max_reach) {
    x *= max_reach / r;
    y *= max_reach / r;
    r = max_reach;
  } else if (r < min_reach && r > 1e-6f) { // éviter div/0
    x *= min_reach / r;
    y *= min_reach / r;
    r = min_reach;
  }

  // Angle du genou (theta en radian)
  float cos_theta = (r * r - d1 * d1 - d2 * d2) / (2.0f * d1 * d2);
  if (cos_theta > 1.0f) cos_theta = 1.0f;
  if (cos_theta < -1.0f) cos_theta = -1.0f;
  float theta = acos(cos_theta);

  // Limitation du genou
  if (theta > theta_max) {
    theta = theta_max;

    float effective_r = sqrt(d1 * d1 + d2 * d2 + 2.0f * d1 * d2 * cos(theta));
    if (r > 1e-6f) {
      x *= effective_r / r;
      y *= effective_r / r;
    }
    r = effective_r;
  }

  // Angle vers la cible
  float alpha = atan2(y, x);

  // Calcul de beta
  float cos_beta = (r * r + d2 * d2 - d1 * d1) / (2.0f * r * d2);
  if (cos_beta > 1.0f) cos_beta = 1.0f;
  if (cos_beta < -1.0f) cos_beta = -1.0f;
  float beta = acos(cos_beta);

  // Angle du 1er segment (radians → degrés)
  float phi = (alpha - beta) * 180.0f / M_PI;
  theta = theta * 180.0f / M_PI;

  AngleMoteur result = {phi, theta};
  return result;
}

MaxAngleMoteur compute_max_angle(AngleMoteur current, AngleMoteur target, float max_angle) {
    MaxAngleMoteur result;
    
    float diff1 = target.phi - current.phi;
    float diff2 = target.theta - current.theta;

    if (fabs(diff1) > fabs(diff2)) {
        result.maxAngle_phi = max_angle;
        result.maxAngle_theta = fabs(diff2 / diff1) * max_angle;
    } else {
        result.maxAngle_theta = max_angle;
        result.maxAngle_phi = fabs(diff1 / diff2) * max_angle;
    }

    return result;
}

DataFunction interpolate_data_point(DataPoint (*func)(float), float a,float d1, float d2, float theta_max_deg, AngleMoteur actualAngles) {
    DataPoint p1 = func(a);
    DataFunction result;
    
    AngleMoteur angles = inverse_kinematics_2d_flexible(p1.x, p1.y, d1, d2, theta_max_deg);
    MaxAngleMoteur maxAngles = compute_max_angle(actualAngles, angles, p1.maxAngle);

    result.maxAngle_phi = maxAngles.maxAngle_phi;
    result.maxAngle_theta = maxAngles.maxAngle_theta;
    result.phi = angles.phi;
    result.theta = angles.theta;


    return result;
}