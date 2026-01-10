#pragma once

#include "../IShader.h"

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

    bool fragment(const echo::vec3f& barycentric, const echo::face &face, echo::vec4f& fragColour) override
    {
        double current = getTimeSeconds();
        double dt = current - previous;
        previous = current;

        offset += dt;

        grassTexture->offset.x = offset;
        grassTexture->offset.y = offset;
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