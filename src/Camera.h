#pragma once

extern "C"
{
    #include "os/input.h"
}
#include "Renderer/math.hpp"

class Camera
{
struct Plane
{
    vec3f normal;
    float distance;

    float distanceToPoint(const vec3f &point) const
    {
        return (dot(normal, point)) - distance;
    }
};

using Frustum = std::array<Plane, 6>;

public:
    Camera() = default;
    Camera(float fov, float aspect, float zNear, float zFar);
    Camera(int32_t left, int32_t top, int32_t right, int32_t bottom, float zNear, float zFar);

    void updateView();
    void updateRotation(input_ &input);
    void updateMovement(input_ &input, double dt);

    bool isSphereInsideFrustum(const mat4& model, const vec3f& centre, float radius);

    mat4& view()                   { return m_view; }
    mat4& projection()             { return m_projection; }
    const vec3f& position()  const { return m_position; }
    const vec3f& forward()   const { return m_forward; }

    float farPlane()  { return m_farPlane;  }
    float nearPlane() { return m_nearPlane; }

    vec3f m_position  = { 0.f, 0.f, -10.f };
    vec3f m_forward   = { 0.f, 0.f, -1.f };
    float m_yaw         = -90.f;
    float m_pitch       = 0.f;

private:
    mat4 m_projection = identity();
    mat4 m_view       = identity();
    float m_farPlane    = 0.f;
    float m_nearPlane   = 0.f;
    float m_sensitivity = 0.1f;
    float m_cameraSpeed = 5.f;

private:
    Frustum extractFrustumPlanes(const mat4& modelView);
};