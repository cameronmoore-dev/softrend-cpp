#pragma once

#include "IShader.h"

class DepthShader : public echo::IShader
{
public:
    echo::mat4 model;
    echo::mat4 lightSpaceMatrix;

    void vertex(float *vertex) override
    {
        echo::vec3f position = getVertexAttribData<echo::vec3f>(vertex, 0);
        glPosition(0, vertex, lightSpaceMatrix * (model * vec4(position, 1.f)));
    }

    bool fragment(const echo::vec3f& barycentric, const echo::Face &face, echo::vec4f& fragColour) override
    {
        return true;
    }
};