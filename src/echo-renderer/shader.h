#pragma once

#include <memory>

#include "echo_math.h"
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
            uint32_t size;
            uint32_t stride;
        } vertexdef_t;
    
        inline vertexdef_t vertexdef = {};
    }

    enum class WrapMode
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder
    };

    struct MipMap
    {
        std::vector<uint32_t> data;
        uint32_t width;
        uint32_t height;
    };
    
    struct Texture
    {
        std::vector<MipMap> mipLevels;
        vec2f offset = { 0.f, 0.f };
        vec2f scale  = { 1.f, 1.f };
        uint32_t numChannels;
        WrapMode wrapMode = WrapMode::Repeat;
    };
    
    using vertex = float*;
    struct Face
    {
        vertex v0, v1, v2;
    };
    
    class IShader
    {
        friend class Renderer;

        uint32_t m_vtxPosAttribIndex = 0;
    
    protected:
        template<typename T>
        using Uniform = std::unique_ptr<T>;
    
        float _offset = 0.f;
        double _previous = getTimeSeconds();
    
        uint32_t vertID = 0;
        uint32_t fragID = 0;
        
    public:
        template<typename T>
        void setUniform(Uniform<T> &uniform, T &data)
        {
            uniform.reset(&data);
        }
    
        virtual void vertex(float *vertex) = 0;
        virtual bool fragment(const vec3f &barycentric, const Face &face, vec4f &fragColour) = 0;
    
    private:
        void reset();
        void derivatives(const int uvIndex, const Face &face, const Texture &texture, float& dudx, float& dudy, float& dvdx, float& dvdy);
        float lod(const float dudx, const float dudy, const float dvdx, const float dvdy);
        vec2f perspectiveCorrectedUV(const int uvIndex, const Face &face, const vec3f& barycentric);
        uint32_t textureWrap(float &u, float &v, const Texture &texture, const uint8_t mipLevel);
        // const uint32_t bilinear(float u, float v, const MipMap &mip) const;
    
    protected:
        vec3f reflect(const vec3f& i, const vec3f& n);
        vec4f texture(const int uvIndex, const Face &face, Texture &texture, const vec3f& barycentric);
        void glPosition(const int posIndex, float *position, const vec4f &clip);

        template<typename T>
        std::array<T, 3> getFaceAttrib(const Face &face, const int attribIndex)
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
    };
}