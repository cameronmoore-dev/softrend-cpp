#pragma once

#include "echomath.h"
#include "jawl/jawl.hpp"

class Camera
{
    struct Plane
    {
        echo::vec3f normal;
        float distance;

        float distanceToPoint(const echo::vec3f &point) const
        {
            return (echo::dot(normal, point)) - distance;
        }
    };

    using Frustum = std::array<Plane, 6>;

public:
    Camera() = default;
    Camera(float fov, float aspect, float zNear, float zFar);
    Camera(int32_t left, int32_t top, int32_t right, int32_t bottom, float zNear, float zFar);

    void updateView();
    void updateRotation(int xOffset, int yOffset);
    void updateRotation(float xOffset, float yOffset, double dt);
    void updateMovement(double dt);

    bool isSphereInsideFrustum(const echo::mat4& model, const echo::vec3f& centre, float radius);

    echo::mat4& view()                   { return m_view; }
    echo::mat4& projection()             { return m_projection; }
    const echo::vec3f& position()  const { return m_position; }
    const echo::vec3f& forward()   const { return m_forward; }

    float farPlane()  { return m_farPlane;  }
    float nearPlane() { return m_nearPlane; }

private:
    echo::mat4 m_projection = echo::identity();
    echo::mat4 m_view       = echo::identity();

    echo::vec3f m_position  = { 0.f, 0.f, -10.f };
    echo::vec3f m_forward   = { 0.f, 0.f, -1.f };

    float m_farPlane    = 0.f;
    float m_nearPlane   = 0.f;
    
    float m_yaw         = -90.f;
    float m_pitch       = 0.f;
    float m_sensitivity = 0.1f;
    float m_cameraSpeed = 5.f;

private:
    Frustum extractFrustumPlanes(const echo::mat4& modelView);
};