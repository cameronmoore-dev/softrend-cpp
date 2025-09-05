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
            return -(Utils::dot(normal, point)) + distance;
        }
    };

    using Frustrum = std::array<Plane, 6>;

public:
    Camera(float fov, float aspect, float zNear, float zFar);
    Camera(int32_t left, int32_t top, int32_t right, int32_t bottom, float zNear, float zFar);

    void updateView();
    void updateRotation(int xOffset, int yOffset, double dt);
    void updateMovement(double dt);

    bool isSphereInsideFrustrum(const Vec3f& centre, float radius);

    const Mat4& view()       const { return m_view; }
    const Mat4& projection() const { return m_projection; }
    const Vec3f& position()  const { return m_position; }
    const Vec3f& forward()   const { return m_forward; }

    bool isMovingForward    = false;
    bool isMovingLeft       = false;
    bool isMovingRight      = false;
    bool isMovingBackward   = false;
    bool isMovingDown       = false;
    bool isMovingUp         = false;

private:
    Mat4 m_projection = Matrix::identity();
    Mat4 m_view       = Matrix::identity();

    Vec3f m_position  { 0, 0, -5 };
    Vec3f m_forward   { 0, 0, 1 };

    // frustrum culling

    float m_yaw         = -90.f;
    float m_pitch       = 0.f;
    float m_sensitivity = 0.1f;
    float m_cameraSpeed = 5.f;

private:
    Frustrum extractFrustrumPlanes();
};