#pragma once

#include "../IShader.h"

class Window_Shader : public echo::IShader
{
public:
    Uniform<echo::mat4> model;
    Uniform<echo::mat4> view;
    Uniform<echo::mat4> projection;

    Uniform<echo::Texture> windowTexture;

    float offset = 0.f;

    void vertex(float *vertex) override
    {
        echo::vec3f position = getVertexAttribData<echo::vec3f>(vertex, 0);

        echo::vec4f clip = *projection * (*view * (*model * vec4(position, 1.f)));
        glPosition(0, vertex, clip);
    }

    bool fragment(const echo::vec3f& barycentric, const echo::Face &face, echo::vec4f& fragColour) override
    {
        double current = getTimeSeconds();
        double dt = current - _previous;
        _previous = current;

        _offset += dt;

        windowTexture->offset.x = -_offset;
        windowTexture->offset.y = -_offset;
        windowTexture->scale.x = 1.5f;
        windowTexture->scale.y = 1.5f;

        fragColour = texture(2, face, *windowTexture, barycentric);
        return true;
    }
};