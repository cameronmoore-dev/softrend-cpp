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

    if (Input::isGamepadConnected(0))
    {
        Vec3f f = (m_forward * Input::controllerAxisValue(0, Input::JAWL_GAMEPAD_AXIS::LY));
        Vec3f r = (right     * Input::controllerAxisValue(0, Input::JAWL_GAMEPAD_AXIS::LX));
        m_position -= f * m_cameraSpeed * dt;
        m_position -= r * m_cameraSpeed * dt;
        m_position.y += Input::isGamepadButtonPressed(0, Input::JAWL_GAMEPAD_BUTTONS::LEFT_SHOULDER)  * m_cameraSpeed * dt;
        m_position.y -= Input::isGamepadButtonPressed(0, Input::JAWL_GAMEPAD_BUTTONS::RIGHT_SHOULDER) * m_cameraSpeed * dt;

        return;
    }

    Vec3f f = m_forward * ((int)Input::isKeyPressed(Input::JAWL_KEY::W) - (int)Input::isKeyPressed(Input::JAWL_KEY::S));
    Vec3f r = right     * ((int)Input::isKeyPressed(Input::JAWL_KEY::D) - (int)Input::isKeyPressed(Input::JAWL_KEY::A));
    Vec3f u = worldUp   * ((int)Input::isKeyPressed(Input::JAWL_KEY::Q) - (int)Input::isKeyPressed(Input::JAWL_KEY::E));

    m_position -= f * m_cameraSpeed * dt;
    m_position -= r * m_cameraSpeed * dt;
    m_position -= u * m_cameraSpeed * dt;
}