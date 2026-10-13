#include "Camera.h"

Camera::Camera(float fov, float aspect, float zNear, float zFar)
{
    m_projection = perspective(fov, aspect, zNear, zFar);
    m_farPlane = zFar;
    m_nearPlane = zNear;
}

Camera::Camera(int32_t left, int32_t top, int32_t right, int32_t bottom, float zNear, float zFar)
{
    m_projection = orthographic(left, top, right, bottom, zNear, zFar);
    m_farPlane = zFar;
    m_nearPlane = zNear;
}

void Camera::updateView()
{
    m_view = lookAt(m_position, m_position + m_forward, { 0, 1, 0 });  
}

Camera::Frustum Camera::extractFrustumPlanes(const mat4 &modelView)
{
    mat4 vp = m_projection * modelView;
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

    for (Plane &p : planes)
    {
        float l = length(p.normal);
        p.normal /= l;
        p.distance /= l;
    }

    return planes;
}

bool Camera::isSphereInsideFrustum(const mat4& model, const vec3f& centre, float radius)
{
    mat4 modelView = m_view * model;
    Frustum planes = extractFrustumPlanes(modelView);

    for (const Plane &p : planes)
    {
        if (p.distanceToPoint(centre) > radius)
        {
            return false;
        }
    }

    return true;
}

void Camera::updateRotation(input_ &input)
{
    m_yaw   += input.mouse_dx * m_sensitivity;
    m_pitch -= input.mouse_dy * m_sensitivity;
    m_pitch = std::clamp(m_pitch, -89.f, 89.f);

    input.mouse_dx = 0;
    input.mouse_dy = 0;

    float rad_yaw = degToRad(m_yaw);
    float rad_pitch = degToRad(m_pitch);

    vec3f dir;
    dir.x = std::cos(rad_yaw) * std::cos(rad_pitch);
    dir.y = std::sin(rad_pitch);
    dir.z = std::sin(rad_yaw) * std::cos(rad_pitch);

    m_forward = normalise(dir);
}

void Camera::updateMovement(input_ &input, double dt)
{
    vec3f worldUp = { 0, 1, 0 };
    vec3f right = normalise(cross(m_forward, worldUp));

    vec3f f = m_forward * (float)((int)is_key_held(&input, KEY_W) - (int)is_key_held(&input, KEY_S));
    vec3f r = right     * (float)((int)is_key_held(&input, KEY_D) - (int)is_key_held(&input, KEY_A));
    vec3f u = worldUp   * (float)((int)is_key_held(&input, KEY_Q) - (int)is_key_held(&input, KEY_E));

    m_position -= f * m_cameraSpeed * (float)dt;
    m_position -= r * m_cameraSpeed * (float)dt;
    m_position -= u * m_cameraSpeed * (float)dt;
}