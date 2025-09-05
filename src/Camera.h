#pragma once

#include "Matrix.h"
#include "Utils.h"

class Camera
{
public:
    Camera();

    void updateRotation(int xOffset, int yOffset, double dt);
    void updateMovement(double dt);
    
    Vec3f position  { 0, 0, -5 };
    Vec3f up        { 0, 1, 0 };
    Vec3f forward   { 0, 0, 1 };

    bool isMovingForward    = false;
    bool isMovingLeft       = false;
    bool isMovingRight      = false;
    bool isMovingBackward   = false;
    bool isMovingDown       = false;
    bool isMovingUp         = false;

private:
    float m_yaw = -90.f;
    float m_pitch = 0.f;
    float m_sensitivity = 10.f;
    float m_cameraSpeed = 5.f;
};