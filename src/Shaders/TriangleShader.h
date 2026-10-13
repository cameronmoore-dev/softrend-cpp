#pragma once

#include "Renderer/shader.h"

class TriangleShader : public IShader
{
public:
    void vertex(float *vertex) override
    {
        vec3f position = getVertexAttribData<vec3f>(vertex, 0);
        glPosition(0, vertex, vec4(position, 1.0f));
    }

    bool fragment(const vec3f& barycentric, const Face &face, vec4f& fragColour) override
    {
        vec3f v0_colour = getVertexAttribData<vec3f>(face.v0, 5);
        vec3f v1_colour = getVertexAttribData<vec3f>(face.v1, 5);
        vec3f v2_colour = getVertexAttribData<vec3f>(face.v2, 5);

        fragColour.x = (v0_colour.x * barycentric.x) + (v1_colour.x * barycentric.y) + (v2_colour.x * barycentric.z);
        fragColour.y = (v0_colour.y * barycentric.x) + (v1_colour.y * barycentric.y) + (v2_colour.y * barycentric.z);
        fragColour.z = (v0_colour.z * barycentric.x) + (v1_colour.z * barycentric.y) + (v2_colour.z * barycentric.z);
        fragColour.w = 1.0f;
        return true;
    }
};