#pragma once

#include <array>
#include <stdint.h>

#include "Matrix.h"
#include "Utils.h"

class Camera
{
    struct Plane
    {
        Vec3f normal;
        float distance;

        float distanceToPoint(const Vec3f& point) const
        {
            return (Utils::dot(normal, point)) - distance;
        }
    };

    using Frustum = std::array<Plane, 6>;

public:
    Camera(float fov, float aspect, float zNear, float zFar);
    Camera(int32_t left, int32_t top, int32_t right, int32_t bottom, float zNear, float zFar);

    void updateView();
    void updateRotation(int xOffset, int yOffset, double dt);
    void updateMovement(double dt);

    bool isSphereInsideFrustum(const Mat4& model, const Vec3f& centre, float radius);

    const Mat4& view()       const { return m_view; }
    const Mat4& projection() const { return m_projection; }
    const Vec3f& position()  const { return m_position; }
    const Vec3f& forward()   const { return m_forward; }

    const float farPlane()   const { return m_farPlane; }
    const float nearPlane()  const { return m_nearPlane; }

    bool isMovingForward    = false;
    bool isMovingLeft       = false;
    bool isMovingRight      = false;
    bool isMovingBackward   = false;
    bool isMovingDown       = false;
    bool isMovingUp         = false;

private:
    Mat4 m_projection = Matrix::identity();
    Mat4 m_view       = Matrix::lookAt(m_position, m_position + m_forward, {0, 1, 0});

    Vec3f m_position  { 0, 0, -5 };
    Vec3f m_forward   { 0, 0, -1 };

    float m_farPlane    = 0.f;
    float m_nearPlane   = 0.f;
    
    float m_yaw         = -90.f;
    float m_pitch       = 0.f;
    float m_sensitivity = 0.1f;
    float m_cameraSpeed = 5.f;

private:
    Frustum extractFrustumPlanes(const Mat4& modelView);
};