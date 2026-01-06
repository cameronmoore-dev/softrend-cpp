#pragma once

#include "../IShader.h"

class Cube_Shader : public echo::IShader
{
public:
    Uniform<echo::mat4> model;
    Uniform<echo::mat4> view;
    Uniform<echo::mat4> projection;

    Uniform<echo::Texture> texture;

    void vertex(float *vertex) override
    {
        // copy.uv = v.uv;
        // copy.normal = v.normal;
        // copy.position = Matrix::multiply(*projection, Matrix::multiply(*view, Matrix::multiply(*model, v.position)));
    }

    bool fragment(const echo::vec3f& barycentric, const echo::face &face, echo::vec4f& finalColour) override
    {
        // finalColour = sampleDiffuseMap(polygon, *texture, barycentric);
        return true;
    }
};