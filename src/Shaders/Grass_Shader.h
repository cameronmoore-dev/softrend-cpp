#pragma once

#include "echo-renderer/shader.h"

class Grass_Shader : public echo::IShader
{
public:
    Uniform<echo::mat4> projection;
    Uniform<echo::mat4> view;
    Uniform<echo::mat4> model;

    Uniform<echo::Texture> grassTexture;

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

        grassTexture->offset.x = _offset;
        grassTexture->offset.y = _offset;
        grassTexture->scale.x = 2;

        echo::vec4f texel = texture(2, face, *grassTexture, barycentric);
        if (texel.w == 0)
        {
            return false;
        }

        fragColour = texel;
        return true;
    }
};