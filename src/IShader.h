#pragma once

#include <algorithm>

#include "Structs.h"
#include "Utils.h"
#include "Image.h"

class IShader
{
public:
    virtual void vertex(const Vertex& v, glVertex& copy) = 0;
    virtual uint32_t fragment(const Vec3f& barycentric, const glTriangle& polygon, float depth) = 0;

private:
    Vec2i perspectiveCorrectedUV(const glTriangle& polygon, const Image& image, const Vec3f& barycentric)
    {
        /* Perspective-correct interpolation */
        float invInterp =       polygon.v0.inverseW * barycentric.x +
                                polygon.v1.inverseW * barycentric.y +
                                polygon.v2.inverseW * barycentric.z;

        Vec2f uvOverZInterp =   polygon.v0.uv * barycentric.x +
                                polygon.v1.uv * barycentric.y +
                                polygon.v2.uv * barycentric.z;

        Vec2f finalUV = uvOverZInterp / invInterp;

        int32_t u = finalUV.x * (image.size().x - 1);
        int32_t v = finalUV.y * (image.size().y - 1);

        u = std::clamp(u, 0, image.size().x - 1);
        v = std::clamp(v, 0, image.size().y - 1);

        return { u, v };
    }

protected:
    uint32_t sampleDiffuseMap(const glTriangle& polygon, const Image& image, const Vec3f& barycentric)
    {
        Vec2i uv = perspectiveCorrectedUV(polygon, image, barycentric);
        return image.pixel(uv.x + uv.y * image.size().x);
    }

    Vec3f sampleNormalMap(const glTriangle& polygon, const Image& image, const Vec3f& barycentric)
    {
        // Perspective-correct TBN
        Vec3f T = polygon.v0.tangent * barycentric.x + polygon.v1.tangent * barycentric.y + polygon.v2.tangent * barycentric.z;
        Vec3f B = polygon.v0.bitangent * barycentric.x + polygon.v1.bitangent * barycentric.y + polygon.v2.bitangent * barycentric.z;
        Vec3f N = polygon.v0.normal * barycentric.x + polygon.v1.normal * barycentric.y + polygon.v2.normal * barycentric.z;

        Vec2i uv = perspectiveCorrectedUV(polygon, image, barycentric);
        Vec4i texel = image.pixelRGBA(uv.x + uv.y * image.size().x);
        Vec3f normal;
        normal.x = (texel.x / 255.f) * 2.f - 1.f;
        normal.y = (texel.y / 255.f) * 2.f - 1.f;
        normal.z = (texel.z / 255.f) * 2.f - 1.f;
        normal = Utils::normalise(normal);

        Mat3 tbn = Matrix::constructTBN(T, B, N);

        return Utils::normalise(Matrix::multiplyVec3(tbn, normal));
    }
};