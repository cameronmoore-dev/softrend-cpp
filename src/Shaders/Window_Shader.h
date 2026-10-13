#pragma once

#include "Renderer/shader.h"

class Window_Shader : public IShader
{
public:
    Uniform<mat4> model;
    Uniform<mat4> view;
    Uniform<mat4> projection;

    Uniform<Texture> windowTexture;

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

        offset += (f32)dt.us / 1000000L;

        windowTexture->offset.x = -_offset;
        windowTexture->offset.y = -_offset;
        windowTexture->scale.x = 1.5f;
        windowTexture->scale.y = 1.5f;

        fragColour = texture(2, face, *windowTexture, barycentric);
        return true;
    }
};