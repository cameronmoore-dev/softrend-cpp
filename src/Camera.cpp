#include "Camera.h"

#include <algorithm>
#include <cmath>
#include <stdio.h>

Camera::Camera()
{
}

void Camera::updateRotation(float xOffset, float yOffset)
{
    m_yaw += xOffset * m_sensitivity;
    m_pitch += yOffset * m_sensitivity;

    m_pitch = std::clamp(m_pitch, -89.f, 89.f);

    float rad_yaw = Utils::degToRad(m_yaw);
    float rad_pitch = Utils::degToRad(m_pitch);

    Vec3f dir;
    dir.x = std::cos(rad_yaw) * std::cos(rad_pitch);
    dir.y = std::sin(rad_pitch);
    dir.z = std::sin(rad_yaw) * std::cos(rad_pitch);

    forward = Utils::normalise(dir);

    printf("forward: %f, %f, %f\n", forward.x, forward.y, forward.z);
}