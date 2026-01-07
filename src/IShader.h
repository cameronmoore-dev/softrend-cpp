#pragma once

#include <algorithm>
#include <memory>

#include "math.h"
#include "jawl/base_window.h"

namespace echo
{
    namespace _priv
    {
        typedef struct 
        {
            int index;
            int count;
        } attrib_t;
    
        typedef struct
        {
            std::vector<attrib_t> attribs;
            uint16_t size;
            uint8_t stride;
        } vertexdef_t;
    
        inline vertexdef_t vertexdef = {};
    }

    struct MipMap
    {
        std::vector<uint32_t> data;
        uint16_t width;
        uint16_t height;
    };
    
    struct Texture
    {
        std::vector<MipMap> mipLevels;
        vec2f offset = {0.f, 0.f};
        vec2f scale  = {1.f, 1.f};
        uint8_t numChannels;
    };
    
    using vertex = float*;
    typedef struct 
    {
        vertex v0, v1, v2;
    } face;
    
    class IShader
    {
        friend class Renderer;
    
    protected:
        template<typename T>
        using Uniform = std::unique_ptr<T>;
    
        float offset = 0.f;
        double previous = getTimeSeconds();
    
        uint32_t vertID = 0;
        uint32_t fragID = 0;
        
    public:
        // virtual ~IShader() = 0;
    
        template<typename T>
        void setUniform(Uniform<T> &uniform, T &data)
        {
            uniform.reset(&data);
        }
    
        virtual void vertex(float *vertex) = 0;
        virtual bool fragment(const vec3f &barycentric, const face &face, vec4f &fragColour) = 0;
    
    private:
        void reset();
        void derivatives(const int uvIndex, const face &face, const Texture &texture, float& dudx, float& dudy, float& dvdx, float& dvdy);
        float lod(float dudx, float dudy, float dvdx, float dvdy);
        vec2f perspectiveCorrectedUV(const int uvIndex, const face &face, const vec3f& barycentric);
        uint32_t textureWrap(float &u, float &v, const Texture &texture, const uint8_t mipLevel);
        // const uint32_t bilinear(float u, float v, const MipMap &mip) const;
    
    protected:
        vec3f reflect(const vec3f& i, const vec3f& n);
        vec4f texture(const int uvIndex, const face &face, Texture &texture, const vec3f& barycentric);
        void glPosition(int posIndex, float *position, const vec4f &clip);

        template<typename T>
        std::array<T, 3> getFaceAttrib(const face &face, const int attribIndex)
        {
            return
            { 
                getVertexAttribData<T>(&face.v0[0], attribIndex),
                getVertexAttribData<T>(&face.v1[0], attribIndex), 
                getVertexAttribData<T>(&face.v2[0], attribIndex)
            };
        }

        template<typename T>
        T getVertexAttribData(const float *vertex, const int attribIndex)
        {
            int offset = 0;
            for (int i = 0; i < attribIndex; i++)
            {
                offset += _priv::vertexdef.attribs[i].count;
            }

            if constexpr(std::is_same_v<T, vec4f>)
            {
                return vec4f{ vertex[offset], vertex[offset + 1], vertex[offset + 2], vertex[offset + 3] };
            }
            else if constexpr(std::is_same_v<T, vec3f>)
            {
                return vec3f{ vertex[offset], vertex[offset + 1], vertex[offset + 2] };
            }
            else
            {
                return vec2f{ vertex[offset], vertex[offset + 1] };
            }
        }
    
        // Vec4i sampleRGBA(const glTriangle& polygon, Image& image, const vec3f& barycentric)
        // {
        //     double current = getTimeSeconds();
        //     double dt = current - previous;
        //     previous = current;
    
    
        //     float dudx, dudy, dvdx, dvdy;
        //     image.derivatives(polygon, dudx, dudy, dvdx, dvdy);
    
        //     float lod = image.lod(dudx, dudy, dvdx, dvdy);
    
        //     vec2f uv = perspectiveCorrectedUV(polygon, barycentric);
    
        //     offset += dt;
        //     uv += offset;
    
        //     return image.pixelRGBA(uv.x, uv.y, lod);
        // }
    };
}