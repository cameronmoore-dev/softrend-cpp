#pragma once

#include <cassert>
#include <iostream>

#include "../IShader.h"

class Cube_Shader : public IShader
{
public:
    Attribute<Mat4> model;
    Attribute<Mat4> view;
    Attribute<Mat4> projection;

    Attribute<Image> texture;

    void vertex(const Vertex& v, glVertex& copy) override
    {
        copy.uv = v.uv;
        copy.normal = v.normal;
        copy.position = Matrix::multiply(*projection, Matrix::multiply(*view, Matrix::multiply(*model, v.position)));
    }

    bool fragment(const Vec3f& barycentric, const glTriangle& polygon, Vec4f& finalColour) override
    {
        Vec4i texel = sampleRGBA(polygon, *texture, barycentric);
        finalColour = Vec4f{ (float)texel.x / 255.f, (float)texel.y / 255.f, (float)texel.z / 255.f, (float)texel.w / 255.f };
        return true;
    }
};