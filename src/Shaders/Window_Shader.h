#pragma once

#include "../IShader.h"

#include <iostream>

class Window_Shader : public IShader
{
public:
    Mat4* model = nullptr;
    Mat4* view = nullptr;
    Mat4* projection = nullptr;

    Image* texture = nullptr;

    void vertex(const Vertex& v, glVertex& copy) override
    {
        copy.uv = v.uv;
        copy.position = Matrix::multiply(*projection, Matrix::multiply(*view, Matrix::multiply(*model, v.position)));
    }

    bool fragment(const Vec3f& barycentric, const glTriangle& polygon, Vec4f& finalColour) override
    {
        Vec4i texel = sampleRGBA(polygon, *texture, barycentric);

        finalColour = Vec4f{ (float)texel.x / 255.f, (float)texel.y / 255.f, (float)texel.z / 255.f, (float)texel.w / 255.f };
        return true;
    }
};