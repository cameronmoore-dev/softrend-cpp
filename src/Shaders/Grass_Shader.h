#pragma once

#include "../IShader.h"

class Grass_Shader : public IShader
{
public:
    Uniform<Mat4> projection;
    Uniform<Mat4> view;
    Uniform<Mat4> model;

    Uniform<Texture> texture;

    void vertex(float *vertex) override
    {
        // copy.uv = v.uv;
        // copy.position = Matrix::multiply(*projection, Matrix::multiply(*view, Matrix::multiply(*model, v.position)));
    }

    bool fragment(const Vec3f& barycentric, const face_t &face, Vec4f& finalColour) override
    {
        // Vec4f texel = sampleDiffuseMap(polygon, *texture, barycentric);
        // if (texel.w == 0)
        // {
        //     return false;
        // }

        // finalColour = texel;
        return true;
    }
};