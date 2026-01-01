#pragma once

#include <algorithm>
#include <memory>

#include "math.h"
#include "Structs.h"
#include "Image.h"

#include "jawl/base_window.h"

struct MipMap
{
    std::vector<uint32_t> data;
    uint16_t width;
    uint16_t height;
};

struct Texture
{
    std::vector<MipMap> mipLevels;
};

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
    void derivatives(const glTriangle& polygon, Texture &texture, float& dudx, float& dudy, float& dvdx, float& dvdy);
    float lod(float dudx, float dudy, float dvdx, float dvdy);
    Vec2f perspectiveCorrectedUV(const glTriangle& polygon, const Vec3f& barycentric);

protected:
    float dot(const Vec3f& a, const Vec3f& b);
    float length(const Vec3f& normal);
    Vec3f cross(const Vec3f& a, const Vec3f& b);
    Vec3f reflect(const Vec3f& i, const Vec3f& n);
    Vec3f normalise(const Vec3f& vec);
    Vec4f vec4(const Vec3f &vec, const float w);
    Vec4f vec4(const Vec2f &vec, const float z, const float w);

    const float srgbToLinear(const uint8_t channel);
    const uint32_t bilinear(float u, float v, const MipMap &mip) const;
    Vec4f pixelRGBA(const Texture &texture, float u, float v, float lod);
    Vec3f pixelRGB(const Texture &texture, float u, float v, float lod);
    Vec4f sampleDiffuseMap(const glTriangle& polygon, Texture &texture, const Vec3f& barycentric);
    Vec3f sampleNormalMap(const glTriangle& polygon, Texture &texture, const Vec3f& barycentric);
    Vec3f sampleSpecularMap(const glTriangle& polygon, Texture &texture, const Vec3f& barycentric);

    // Vec4i sampleRGBA(const glTriangle& polygon, Image& image, const Vec3f& barycentric)
    // {
    //     double current = getTimeSeconds();
    //     double dt = current - previous;
    //     previous = current;


    //     float dudx, dudy, dvdx, dvdy;
    //     image.derivatives(polygon, dudx, dudy, dvdx, dvdy);

    //     float lod = image.lod(dudx, dudy, dvdx, dvdy);

    //     Vec2f uv = perspectiveCorrectedUV(polygon, barycentric);

    //     offset += dt;
    //     uv += offset;

    //     return image.pixelRGBA(uv.x, uv.y, lod);
    // }
};