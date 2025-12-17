#pragma once

#include "../IShader.h"

class Grass_Shader : public IShader
{
public:
    Uniform<Mat4> projection;
    Uniform<Mat4> view;
    Uniform<Mat4> model;

    Uniform<Image> texture;

    void vertex(const Vertex& v, glVertex& copy) override
    {
        copy.uv = v.uv;
        copy.position = Matrix::multiply(*projection, Matrix::multiply(*view, Matrix::multiply(*model, v.position)));
    }

    bool fragment(const Vec3f& barycentric, const glTriangle& polygon, Vec4f& finalColour) override
    {
        Vec4i texel = sampleRGBA(polygon, *texture, barycentric);
        if (texel.w == 0)
        {
            return false;
        }

        finalColour = Vec4f{ (float)texel.x / 255.f, (float)texel.y / 255.f, (float)texel.z / 255.f, (float)texel.w / 255.f };
        return true;
    }
};