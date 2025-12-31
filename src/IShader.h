#pragma once

#include <algorithm>
#include <memory>

#include "math.h"
#include "Structs.h"
#include "Image.h"

#include "jawl/base_window.h"

class IShader
{
protected:
    template<typename T>
    using Uniform = std::unique_ptr<T>;

    float offset = 0.f;
    double previous = getTimeSeconds();

public:
    // virtual ~IShader() = 0;

    template<typename T>
    void setUniform(Uniform<T> &uniform, T &data)
    {
        uniform.reset(&data);
    }

    virtual void vertex(const Vertex& v, glVertex& copy) = 0;
    virtual bool fragment(const Vec3f &barycentric, const glTriangle &polygon, Vec4f &fragColour) = 0;

private:
    Vec2f perspectiveCorrectedUV(const glTriangle& polygon, const Image& image, const Vec3f& barycentric)
    {
        /* Perspective-correct interpolation */
        float invInterp =       polygon.v0.inverseW * barycentric.x +
                                polygon.v1.inverseW * barycentric.y +
                                polygon.v2.inverseW * barycentric.z;

        Vec2f uvOverZInterp =   polygon.v0.uv * barycentric.x +
                                polygon.v1.uv * barycentric.y +
                                polygon.v2.uv * barycentric.z;

        Vec2f finalUV = uvOverZInterp / invInterp;

        return { finalUV.x, finalUV.y };
    }

protected:
    float dot(const Vec3f& a, const Vec3f& b);
    float length(const Vec3f& normal);
    Vec3f cross(const Vec3f& a, const Vec3f& b);
    Vec3f reflect(const Vec3f& i, const Vec3f& n);
    Vec3f normalise(const Vec3f& vec);
    Vec4f vec4(const Vec3f &vec, const float w);
    Vec4f vec4(const Vec2f &vec, const float z, const float w);


    Vec3f sampleDiffuseMap(const glTriangle& polygon, Image& image, const Vec3f& barycentric)
    {
        float dudx, dudy, dvdx, dvdy;
        image.derivatives(polygon, dudx, dudy, dvdx, dvdy);

        float lod = image.lod(dudx, dudy, dvdx, dvdy);

        Vec2f uv = perspectiveCorrectedUV(polygon, image, barycentric);
        return image.pixel(uv.x, uv.y, lod);
    }

    Vec4i sampleRGBA(const glTriangle& polygon, Image& image, const Vec3f& barycentric)
    {
        double current = getTimeSeconds();
        double dt = current - previous;
        previous = current;


        float dudx, dudy, dvdx, dvdy;
        image.derivatives(polygon, dudx, dudy, dvdx, dvdy);

        float lod = image.lod(dudx, dudy, dvdx, dvdy);

        Vec2f uv = perspectiveCorrectedUV(polygon, image, barycentric);

        offset += dt;
        uv += offset;

        return image.pixelRGBA(uv.x, uv.y, lod);
    }

    Vec3f sampleNormalMap(const glTriangle& polygon, Image& image, const Vec3f& barycentric)
    {
        // Perspective-correct TBN
        Vec3f T = polygon.v0.tangent * barycentric.x + polygon.v1.tangent * barycentric.y + polygon.v2.tangent * barycentric.z;
        Vec3f B = polygon.v0.bitangent * barycentric.x + polygon.v1.bitangent * barycentric.y + polygon.v2.bitangent * barycentric.z;
        Vec3f N = polygon.v0.normal * barycentric.x + polygon.v1.normal * barycentric.y + polygon.v2.normal * barycentric.z;

        float dudx, dudy, dvdx, dvdy;
        image.derivatives(polygon, dudx, dudy, dvdx, dvdy);
        float lod = image.lod(dudx, dudy, dvdx, dvdy);

        Vec2f uv = perspectiveCorrectedUV(polygon, image, barycentric);
        Vec4i texel = image.pixelRGBA(uv.x, uv.y, lod);
        Vec3f normal;
        normal.x = (texel.x / 255.f) * 2.f - 1.f;
        normal.y = (texel.y / 255.f) * 2.f - 1.f;
        normal.z = (texel.z / 255.f) * 2.f - 1.f;
        normal = normalise(normal);

        Mat3 tbn = Matrix::constructTBN(T, B, N);

        return normalise(Matrix::multiplyVec3(tbn, normal));
    }

    Vec3f sampleSpecularMap(const glTriangle& polygon, Image& image, const Vec3f& barycentric)
    {
        float dudx, dudy, dvdx, dvdy;
        image.derivatives(polygon, dudx, dudy, dvdx, dvdy);

        float lod = image.lod(dudx, dudy, dvdx, dvdy);

        Vec2f uv = perspectiveCorrectedUV(polygon, image, barycentric);
        return image.pixel(uv.x, uv.y, lod);
    }
};