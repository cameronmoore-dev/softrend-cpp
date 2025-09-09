#pragma once

#include <algorithm>

#include "Structs.h"
#include "Image.h"

class IShader
{
public:
    virtual void vertex(const Vertex& v, glVertex& copy) = 0;
    virtual uint32_t fragment(const Vec3f& barycentric, const glTriangle& polygon, float depth) = 0;

protected:
    uint32_t texel(const glTriangle& polygon, const Image& image, const Vec3f& barycentric)
    {
        /* Perspective-correct interpolation */
        float invInterp =       polygon.v0.inverseW * barycentric.x +
                                polygon.v1.inverseW * barycentric.y +
                                polygon.v2.inverseW * barycentric.z;

        Vec2f uvOverZInterp =   polygon.v0.texCoords * barycentric.x +
                                polygon.v1.texCoords * barycentric.y +
                                polygon.v2.texCoords * barycentric.z;

        Vec2f finalUV = uvOverZInterp / invInterp;

        int32_t texX = finalUV.x * (image.size().x - 1);
        int32_t texY = finalUV.y * (image.size().y - 1);

        texX = std::clamp(texX, 0, image.size().x - 1);
        texY = std::clamp(texY, 0, image.size().y - 1);

        return image.pixel(texX + texY * image.size().x);
    }
};