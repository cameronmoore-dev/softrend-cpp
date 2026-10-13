#pragma once

#include "Renderer/shader.h"

class Grass_Shader : public IShader
{
public:
    Uniform<mat4> projection;
    Uniform<mat4> view;
    Uniform<mat4> model;

    Uniform<Texture> grassTexture;

    float offset = 0.f;

    void vertex(float *vertex) override
    {
        vec3f position = getVertexAttribData<vec3f>(vertex, 0);

        vec4f clip = *projection * (*view * (*model * vec4(position, 1.f)));
        glPosition(0, vertex, clip);
    }

    bool fragment(const vec3f& barycentric, const Face &face, vec4f& fragColour) override
    {
        timepoint_ current = get_time();
        duration_ dt = timepoint_diff(&current, &_previous);
        _previous = current;

        _offset += (f32)dt.us / 1000000L;

        grassTexture->offset.x = _offset;
        grassTexture->offset.y = _offset;
        grassTexture->scale.x = 2;

        vec4f texel = texture(2, face, *grassTexture, barycentric);
        if (texel.w == 0)
        {
            return false;
        }

        fragColour = texel;
        return true;
    }
};