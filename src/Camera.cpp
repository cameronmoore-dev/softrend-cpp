#include "Camera.h"

#include <algorithm>
#include <cmath>
#include <stdio.h>

Camera::Camera(float fov, float aspect, float zNear, float zFar)
{
    m_projection = Matrix::perspective(fov, aspect, zNear, zFar);
}

Camera::Camera(int32_t left, int32_t top, int32_t right, int32_t bottom, float zNear, float zFar)
{
    m_projection = Matrix::orthographic(left, top, right, bottom, zNear, zFar);
}

void Camera::updateView()
{
    m_view = Matrix::lookAt(m_position, m_position + m_forward, {0, 1, 0});
}

Camera::Frustrum Camera::extractFrustrumPlanes()
{
    Mat4 vp = Matrix::multiply(m_projection, m_view);
    Frustrum planes;

    /* Left */
    planes[0].normal.x = vp[0][3] + vp[0][0];
    planes[0].normal.y = vp[1][3] + vp[1][0];
    planes[0].normal.z = vp[2][3] + vp[2][0];
    planes[0].distance = vp[3][3] + vp[3][0];

    // Right
    planes[1].normal.x = vp[0][3] - vp[0][0];
    planes[1].normal.y = vp[1][3] - vp[1][0];
    planes[1].normal.z = vp[2][3] - vp[2][0];
    planes[1].distance = vp[3][3] - vp[3][0];

    // Bottom
    planes[2].normal.x = vp[0][3] + vp[0][1];
    planes[2].normal.y = vp[1][3] + vp[1][1];
    planes[2].normal.z = vp[2][3] + vp[2][1];
    planes[2].distance = vp[3][3] + vp[3][1];

    // Top
    planes[3].normal.x = vp[0][3] - vp[0][1];
    planes[3].normal.y = vp[1][3] - vp[1][1];
    planes[3].normal.z = vp[2][3] - vp[2][1];
    planes[3].distance = vp[3][3] - vp[3][1];

    // Near
    planes[4].normal.x = vp[0][3] + vp[0][2];
    planes[4].normal.y = vp[1][3] + vp[1][2];
    planes[4].normal.z = vp[2][3] + vp[2][2];
    planes[4].distance = vp[3][3] + vp[3][2];

    // Far
    planes[5].normal.x = vp[0][3] - vp[0][2];
    planes[5].normal.y = vp[1][3] - vp[1][2];
    planes[5].normal.z = vp[2][3] - vp[2][2];
    planes[5].distance = vp[3][3] - vp[3][2];

    for (Plane& p : planes)
    {
        float length = Utils::length(p.normal);
        p.normal /= length;
        p.distance /= length;
    }

    return planes;
}

bool Camera::isSphereInsideFrustrum(const Vec3f& centre, float radius)
{
    Frustrum planes = extractFrustrumPlanes();
    Vec3f viewCentre = Utils::toVec3(Matrix::multiply(m_view, centre));

    for (const Plane& p : planes)
    {
        if (p.distanceToPoint(viewCentre) < -radius)
        {
            return false;
        }
    }

    return true;
}

void Camera::updateRotation(int xOffset, int yOffset, double dt)
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

    m_forward = Utils::normalise(dir);
}

void Camera::updateMovement(double dt)
{
    Vec3f worldUp{ 0, 1, 0 };

    if (isMovingForward)
    {
        m_position -= m_forward * m_cameraSpeed * dt;
    }

    else if (isMovingBackward)
    {
        m_position += m_forward * m_cameraSpeed * dt;
    }

    if (isMovingLeft)
    {
        m_position += Utils::normalise(Utils::cross(m_forward, worldUp)) * m_cameraSpeed * dt;
    }

    else if (isMovingRight)
    {
        m_position -= Utils::normalise(Utils::cross(m_forward, worldUp)) * m_cameraSpeed * dt;
    }

    if (isMovingUp)
    {
        m_position += worldUp * m_cameraSpeed * dt;
    }

    else if (isMovingDown)
    {
        m_position -= worldUp * m_cameraSpeed * dt;
    }
}