#pragma once

#include "../IShader.h"

#include <iostream>

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

    bool fragment(const echo::vec3f& barycentric, const echo::face &face, echo::vec4f& fragColour) override
    {
        // double current = getTimeSeconds();
        // double dt = current - previous;
        // previous = current;

        // offset += dt;

        // windowTexture->offset.x = -offset;
        // windowTexture->offset.y = -offset;
        // windowTexture->scale.x = 1.5f;
        // windowTexture->scale.y = 1.5f;

        fragColour = texture(2, face, *windowTexture, barycentric);
        return true;
    }
};