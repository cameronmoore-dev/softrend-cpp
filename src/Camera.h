#pragma once

#include <array>
#include <cstdint>

#include "Matrix.h"
#include "math.h"
#include "jawl/jawl.hpp"

class Camera
{
    struct Plane
    {
        Vec3f normal;
        float distance;

        float distanceToPoint(const Vec3f& point) const
        {
            return (math::dot(normal, point)) - distance;
        }
    };

    using Frustum = std::array<Plane, 6>;

public:
    Camera() = default;
    Camera(float fov, float aspect, float zNear, float zFar);
    Camera(int32_t left, int32_t top, int32_t right, int32_t bottom, float zNear, float zFar);

    void updateView();
    void updateRotation(int xOffset, int yOffset, double dt);
    void updateRotation(float xOffset, float yOffset, double dt);
    void updateMovement(double dt);

    bool isSphereInsideFrustum(const Mat4& model, const Vec3f& centre, float radius);

    Mat4& view()                   { return m_view; }
    Mat4& projection()             { return m_projection; }
    const Vec3f& position()  const { return m_position; }
    const Vec3f& forward()   const { return m_forward; }

    const float farPlane()   const { return m_farPlane; }
    const float nearPlane()  const { return m_nearPlane; }

private:
    Mat4 m_projection = Matrix::identity();
    Mat4 m_view       = Matrix::identity();

    Vec3f m_position  = { 0.f, 0.f, -10.f };
    Vec3f m_forward   = { 0.f, 0.f, -1.f };

    float m_farPlane    = 0.f;
    float m_nearPlane   = 0.f;
    
    float m_yaw         = -90.f;
    float m_pitch       = 0.f;
    float m_sensitivity = 0.1f;
    float m_cameraSpeed = 5.f;

private:
    Frustum extractFrustumPlanes(const Mat4& modelView);
};