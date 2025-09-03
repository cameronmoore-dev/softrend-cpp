#pragma once

#include "Matrix.h"
#include "Utils.h"

class Camera
{
public:
    Camera();

    void updateRotation(float xOffset, float yOffset);
    
    Vec3f position{ 0, 0, -5 };
    Vec3f up {0, 1, 0};
    Vec3f forward {0, 0, 1};

private:
    float m_yaw = -90.f;
    float m_pitch = 0.f;
    float m_sensitivity = 0.1f;
};