#include "Camera.h"

#include <algorithm>
#include <cmath>
#include <stdio.h>

Camera::Camera()
{
}

void Camera::updateRotation(int xOffset, int yOffset, double dt)
{
    m_yaw += xOffset * m_sensitivity * dt;
    m_pitch += yOffset * m_sensitivity * dt;

    m_pitch = std::clamp(m_pitch, -89.f, 89.f);

    float rad_yaw = Utils::degToRad(m_yaw);
    float rad_pitch = Utils::degToRad(m_pitch);

    Vec3f dir;
    dir.x = std::cos(rad_yaw) * std::cos(rad_pitch);
    dir.y = std::sin(rad_pitch);
    dir.z = std::sin(rad_yaw) * std::cos(rad_pitch);

    forward = Utils::normalise(dir);
}

void Camera::updateMovement(double dt)
{
    Vec3f worldUp{ 0, 1, 0 };

    if (isMovingForward)
    {
        position -= forward * m_cameraSpeed * dt;
    }

    else if (isMovingBackward)
    {
        position += forward * m_cameraSpeed * dt;
    }

    if (isMovingLeft)
    {
        position += Utils::normalise(Utils::cross(forward, worldUp)) * m_cameraSpeed * dt;
    }

    else if (isMovingRight)
    {
        position -= Utils::normalise(Utils::cross(forward, worldUp)) * m_cameraSpeed * dt;
    }

    if (isMovingUp)
    {
        position += worldUp * m_cameraSpeed * dt;
    }

    else if (isMovingDown)
    {
        position -= worldUp * m_cameraSpeed * dt;
    }
}