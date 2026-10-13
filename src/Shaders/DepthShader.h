#pragma once

#include "Renderer/shader.h"

class DepthShader : public IShader
{
public:
    mat4 model;
    mat4 lightSpaceMatrix;

    void vertex(float *vertex) override
    {
        vec3f position = getVertexAttribData<vec3f>(vertex, 0);
        glPosition(0, vertex, lightSpaceMatrix * (model * vec4(position, 1.f)));
    }

    bool fragment(const vec3f& barycentric, const Face &face, vec4f& fragColour) override
    {
        return true;
    }
};