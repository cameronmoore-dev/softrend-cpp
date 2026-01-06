#pragma once

#include <cassert>
#include <iostream>

#include "../IShader.h"

class Cube_Shader : public IShader
{
public:
    Uniform<Mat4> model;
    Uniform<Mat4> view;
    Uniform<Mat4> projection;

    Uniform<Texture> texture;

    void vertex(float *vertex) override
    {
        // copy.uv = v.uv;
        // copy.normal = v.normal;
        // copy.position = Matrix::multiply(*projection, Matrix::multiply(*view, Matrix::multiply(*model, v.position)));
    }

    bool fragment(const Vec3f& barycentric, const face_t &face, Vec4f& finalColour) override
    {
        // finalColour = sampleDiffuseMap(polygon, *texture, barycentric);
        return true;
    }
};