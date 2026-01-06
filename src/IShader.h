#pragma once

#include <algorithm>
#include <memory>

#include "math.h"
#include "common.h"
#include "Structs.h"

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

using vertex_t  = float*;
typedef struct 
{
    vertex_t v0, v1, v2;
} face_t;

class IShader
{
    friend class Renderer;

    void reset()
    {
        vertIndex = 0;
        fragIndex = 0;
    }

protected:
    template<typename T>
    using Uniform = std::unique_ptr<T>;

    float offset = 0.f;
    double previous = getTimeSeconds();

    uint32_t vertIndex = 0;
    uint32_t fragIndex = 0;
    
public:
    // virtual ~IShader() = 0;

    template<typename T>
    void setUniform(Uniform<T> &uniform, T &data)
    {
        uniform.reset(&data);
    }

    virtual void vertex(float *vertex) = 0;
    virtual bool fragment(const Vec3f &barycentric, const face_t &face, Vec4f &fragColour) = 0;

private:
    void derivatives(const int uvIndex, const face_t &face, Texture &texture, float& dudx, float& dudy, float& dvdx, float& dvdy);
    float lod(float dudx, float dudy, float dvdx, float dvdy);
    Vec2f perspectiveCorrectedUV(const int uvIndex, const face_t &face, const Vec3f& barycentric);

protected:
    float dot(const Vec3f& a, const Vec3f& b);
    float length(const Vec3f& normal);
    Vec3f cross(const Vec3f& a, const Vec3f& b);
    Vec3f reflect(const Vec3f& i, const Vec3f& n);
    Vec3f normalise(const Vec3f& vec);
    Vec3f vec3(const Vec4f &vec);
    Vec4f vec4(const Vec3f &vec, const float w);
    Vec4f vec4(const Vec2f &vec, const float z, const float w);

    const float srgbToLinear(const uint8_t channel);
    const uint32_t bilinear(float u, float v, const MipMap &mip) const;
    Vec4f pixelRGBA(const Texture &texture, float u, float v, float lod);
    Vec3f pixelRGB(const Texture &texture, float u, float v, float lod);
    Vec4f sampleDiffuseMap(const int uvIndex, const face_t &face, Texture &texture, const Vec3f& barycentric);
    Vec3f sampleNormalMap(const face_t &face, Texture &texture, const Vec3f& barycentric);
    Vec3f sampleSpecularMap(const face_t &face, Texture &texture, const Vec3f& barycentric);

    Vec4f pixelRGBA(const Texture &texture, float u, float v);
    Vec3f pixelRGB(const Texture &texture, float u, float v);

    int getVertexStride() { return _priv::vertexdef.stride; }

    std::array<Vec4f, 3> getFaceAttrib4(const face_t &face, const int attribIndex)
    {
        return
        { 
            getVertexAttribData4(&face.v0[0], attribIndex),
            getVertexAttribData4(&face.v1[0], attribIndex), 
            getVertexAttribData4(&face.v2[0], attribIndex)
        };
    }

    std::array<Vec3f, 3> getFaceAttrib(const face_t &face, const int attribIndex)
    {
        return
        { 
            getVertexAttribData3(&face.v0[0], attribIndex), 
            getVertexAttribData3(&face.v1[0], attribIndex), 
            getVertexAttribData3(&face.v2[0], attribIndex)
        };
    }

    Vec4f getVertexAttribData4(const float *vertex, const int attribIndex)
    {
        int offset = 0;
        for (int i = 0; i < attribIndex; i++)
        {
            offset += _priv::vertexdef.attribs[i].count;
        }

        return { vertex[offset], vertex[offset + 1], vertex[offset + 2], vertex[offset + 3] };
    }

    Vec3f getVertexAttribData3(const float *vertex, const int attribIndex)
    {
        int offset = 0;
        for (int i = 0; i < attribIndex; i++)
        {
            offset += _priv::vertexdef.attribs[i].count;
        }

        return { vertex[offset], vertex[offset + 1], vertex[offset + 2] };
    }

    Vec2f getVertexAttribData2(const float *vertex, const int attribIndex)
    {
        int offset = 0;
        for (int i = 0; i < attribIndex; i++)
        {
            offset += _priv::vertexdef.attribs[i].count;
        }

        return { vertex[offset], vertex[offset + 1] };
    }

    void glPosition(int attribIndex, float *position, const Vec4f &clip)
    {
        position[attribIndex]     = clip.x;
        position[attribIndex + 1] = clip.y;
        position[attribIndex + 2] = clip.z;
        position[attribIndex + 3] = clip.w;
        vertIndex++;
    }

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