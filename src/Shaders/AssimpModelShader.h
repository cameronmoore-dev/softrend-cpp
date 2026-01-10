#pragma once

#include "IShader.h"

class AssimpModelShader : public echo::IShader
{
public:
    Uniform<echo::mat4> model;
    Uniform<echo::mat4> view;
    Uniform<echo::mat4> projection;

    Uniform<echo::Texture> diffuse;

    void vertex(float *vertex) override
    {
        echo::vec3f position = getVertexAttribData<echo::vec3f>(vertex, 0);

        echo::vec4f clip = *projection * (*view * (*model * vec4(position, 1.f)));

        glPosition(0, vertex, clip);
    }

    bool fragment(const echo::vec3f& barycentric, const echo::face &face, echo::vec4f& fragColour) override
    {
        fragColour = texture(2, face, *diffuse, barycentric);
        // fragColour = {0.5f,0.5f,0.5f,0.5f};
        return true;
    }
};