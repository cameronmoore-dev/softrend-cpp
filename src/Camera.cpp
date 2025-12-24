#include "Camera.h"

#include <algorithm>
#include <cmath>
#include <stdio.h>

Camera::Camera(float fov, float aspect, float zNear, float zFar)
{
    m_projection = Matrix::perspective(fov, aspect, zNear, zFar);
    m_farPlane = zFar;
    m_nearPlane = zNear;
}

Camera::Camera(int32_t left, int32_t top, int32_t right, int32_t bottom, float zNear, float zFar)
{
    m_projection = Matrix::orthographic(left, top, right, bottom, zNear, zFar);
    m_farPlane = zFar;
    m_nearPlane = zNear;
}

void Camera::updateView()
{
    m_view = Matrix::lookAt(m_position, m_position + m_forward, {0, 1, 0});
}

Camera::Frustum Camera::extractFrustumPlanes(const Mat4& modelView)
{
    Mat4 vp = Matrix::multiply(m_projection, modelView);
    Frustum planes = {};

    /* Gribb & Hartmann method. */
    
    /* Left */
    planes[0].normal.x = vp[3][0] + vp[0][0];
    planes[0].normal.y = vp[3][1] + vp[0][1];
    planes[0].normal.z = vp[3][2] + vp[0][2];
    planes[0].distance = vp[3][3] + vp[0][3];

    /* Right */
    planes[1].normal.x = vp[3][0] - vp[0][0];
    planes[1].normal.y = vp[3][1] - vp[0][1];
    planes[1].normal.z = vp[3][2] - vp[0][2];
    planes[1].distance = vp[3][3] - vp[0][3];

    /* Bottom */
    planes[2].normal.x = vp[3][0] + vp[1][0];
    planes[2].normal.y = vp[3][1] + vp[1][1];
    planes[2].normal.z = vp[3][2] + vp[1][2];
    planes[2].distance = vp[3][3] + vp[1][3];

    /* Top */
    planes[3].normal.x = vp[3][0] - vp[1][0];
    planes[3].normal.y = vp[3][1] - vp[1][1];
    planes[3].normal.z = vp[3][2] - vp[1][2];
    planes[3].distance = vp[3][3] - vp[1][3];

    /* Near */
    planes[4].normal.x = vp[3][0] + vp[2][0];
    planes[4].normal.y = vp[3][1] + vp[2][1];
    planes[4].normal.z = vp[3][2] + vp[2][2];
    planes[4].distance = vp[3][3] + vp[2][3];

    /* Far */
    planes[5].normal.x = vp[3][0] - vp[2][0];
    planes[5].normal.y = vp[3][1] - vp[2][1];
    planes[5].normal.z = vp[3][2] - vp[2][2];
    planes[5].distance = vp[3][3] - vp[2][3];

    for (Plane& p : planes)
    {
        float length = Utils::length(p.normal);
        p.normal /= length;
        p.distance /= length;
    }

    return planes;
}

bool Camera::isSphereInsideFrustum(const Mat4& model, const Vec3f& centre, float radius)
{
    Mat4 modelView = Matrix::multiply(m_view, model);
    Frustum planes = extractFrustumPlanes(modelView);

    for (const Plane& p : planes)
    {
        if (p.distanceToPoint(centre) > radius)
        {
            return false;
        }
    }

    return true;
}

void Camera::updateRotation(int xOffset, int yOffset, double dt)
{
    m_yaw   += xOffset * m_sensitivity;
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

void Camera::updateRotation(float xOffset, float yOffset, double dt)
{
    m_yaw   += xOffset * 100.f * dt;
    m_pitch += yOffset * 100.f * dt;
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
    Vec3f right = Utils::normalise(Utils::cross(m_forward, worldUp));

    if (jawl::isGamepadConnected(0))
    {
        Vec3f f = (m_forward * jawl::controllerAxisValue(0, jawl::GamepadAxis::LY));
        Vec3f r = (right     * jawl::controllerAxisValue(0, jawl::GamepadAxis::LX));
        m_position -= f * m_cameraSpeed * dt;
        m_position -= r * m_cameraSpeed * dt;
        m_position.y += jawl::isGamepadButtonPressed(0, jawl::GamepadButtons::LeftShoulder)  * m_cameraSpeed * dt;
        m_position.y -= jawl::isGamepadButtonPressed(0, jawl::GamepadButtons::RightShoulder) * m_cameraSpeed * dt;

        return;
    }

    Vec3f f = m_forward * ((int)jawl::isKeyPressed(jawl::Key::W) - (int)jawl::isKeyPressed(jawl::Key::S));
    Vec3f r = right     * ((int)jawl::isKeyPressed(jawl::Key::D) - (int)jawl::isKeyPressed(jawl::Key::A));
    Vec3f u = worldUp   * ((int)jawl::isKeyPressed(jawl::Key::Q) - (int)jawl::isKeyPressed(jawl::Key::E));

    m_position -= f * m_cameraSpeed * dt;
    m_position -= r * m_cameraSpeed * dt;
    m_position -= u * m_cameraSpeed * dt;
}