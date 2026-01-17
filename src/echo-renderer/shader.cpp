#include "shader.h"

namespace echo
{
    void IShader::reset()
    {
        vertID = 0;
        fragID = 0;
    }

    vec3f IShader::reflect(const vec3f& i, const vec3f& n)
    {
        return i - n * (2.0f * dot(n, i));
    }
    
    void IShader::derivatives(const int uvIndex, const Face &face, const Texture &texture, float& dudx, float& dudy, float& dvdx, float& dvdy)
    {
        const float texWidth  = (float)texture.mipLevels[0].width;
        const float texHeight = (float)texture.mipLevels[0].height;
    
        std::array<vec4f, 3> positions = getFaceAttrib<vec4f>(face, 0);
        float x1 = positions[1].x - positions[0].x;
        float y1 = positions[1].y - positions[0].y;
        float x2 = positions[2].x - positions[0].x;
        float y2 = positions[2].y - positions[0].y;
    
        vec2f uv0 = getVertexAttribData<vec2f>(&face.v0[0], uvIndex);
        vec2f uv1 = getVertexAttribData<vec2f>(&face.v1[0], uvIndex);
        vec2f uv2 = getVertexAttribData<vec2f>(&face.v2[0], uvIndex);
    
        uv0.x *= texWidth;
        uv0.y *= texHeight;
        uv1.x *= texWidth;
        uv1.y *= texHeight;
        uv2.x *= texWidth;
        uv2.y *= texHeight;
    
        float u1 = uv1.x - uv0.x;
        float v1 = uv1.y - uv0.y;
        float u2 = uv2.x - uv0.x;
        float v2 = uv2.y - uv0.y;
    
        float det = x1 * y2 - x2 * y1;
        if (std::abs(det) < 1e-8f)
        {
            dudx = dudy = dvdx = dvdy = 0;
            return;
        }
    
        float invDet = 1.f / det;
        dudx = (u1 * y2 - u2 * y1) * invDet;
        dudy = (u2 * x1 - u1 * x2) * invDet;
        dvdx = (v1 * y2 - v2 * y1) * invDet;
        dvdy = (v2 * x1 - v1 * x2) * invDet;
    }
    
    float IShader::lod(float dudx, float dudy, float dvdx, float dvdy)
    {
        float dx = dudx * dudx + dvdx * dvdx;
        float dy = dudy * dudy + dvdy * dvdy;
        float rho = std::max(dx, dy);
        return 0.5f * std::log2(rho);
    }
    
    vec2f IShader::perspectiveCorrectedUV(const int uvIndex, const Face &face, const vec3f& barycentric)
    {
        std::array<vec2f, 3> st = getFaceAttrib<vec2f>(face, uvIndex);

        float invW0 = 1.f / face.v0[3];
        float invW1 = 1.f / face.v1[3];
        float invW2 = 1.f / face.v2[3];

        vec2f uv0 = (st[0] / face.v0[3]);
        vec2f uv1 = (st[1] / face.v1[3]);
        vec2f uv2 = (st[2] / face.v2[3]);

        float inverse = invW0 * barycentric.x +
                        invW1 * barycentric.y +
                        invW2 * barycentric.z;

        vec2f uv = uv0 * barycentric.x +
                   uv1 * barycentric.y +
                   uv2 * barycentric.z;

        return uv / inverse;
    }
    
    vec4f IShader::texture(const int uvIndex, const Face &face, Texture &texture, const vec3f& barycentric)
    {
        int32_t mipLevel = 0;
        if (texture.mipLevels.size() > 1)
        {
            float dudx, dudy, dvdx, dvdy;
            derivatives(uvIndex, face, texture, dudx, dudy, dvdx, dvdy);
            float _lod = lod(dudx, dudy, dvdx, dvdy);
            mipLevel = std::clamp((int32_t)(_lod + 0.5f), 0, (int32_t)texture.mipLevels.size() - 1);
        }
        
        vec2f uv = perspectiveCorrectedUV(uvIndex, face, barycentric);
        uint32_t result = textureWrap(uv.x, uv.y, texture, mipLevel);
        vec4f rgba = hexToRGBA(result);

        if (texture.numChannels < 4)
        {
            rgba.w = 1.f;
        }
    
        return rgba;
    }

    uint32_t IShader::textureWrap(float &u, float &v, const Texture &texture, const uint8_t mipLevel)
    {
        u += texture.offset.x;
        v += texture.offset.y;
        u *= texture.scale.x;
        v *= texture.scale.y;

        const MipMap &mip = texture.mipLevels[mipLevel];

        float uFraction = u - (long)u;
        float vFraction = v - (long)v;
        uint32_t x = uint32_t(uFraction * mip.width)  % mip.width;
        uint32_t y = uint32_t(vFraction * mip.height) % mip.height;

        switch (texture.wrapMode)
        {
            case WrapMode::MirroredRepeat:
            {
                if ((int)std::ceil(u) & (1 != 0))
                {
                    x = uint32_t((1.f - uFraction) * mip.width) % mip.width;
                }
    
                if ((int)std::ceil(v) & (1 != 0))
                {
                    y = uint32_t((1.f - vFraction) * mip.height) % mip.height;
                }
            } break;

            case WrapMode::ClampToEdge:
            {
                u = (uint32_t)std::clamp(u, 0.f, 1.f);
                v = (uint32_t)std::clamp(v, 0.f, 1.f);

                x = u * (mip.width - 1);
                y = v * (mip.height - 1);
            } break;

            case WrapMode::ClampToBorder:
            {
                if (u < 0.f || u > 1.f || 
                    v < 0.f || v > 1.f)
                {
                    return 0xffff00ff;
                }
            } break;

            default: break;
        }

        return mip.data[x + y * mip.width];
    }

    void IShader::glPosition(const int posIndex, float *vertex, const vec4f &clip)
    {
        m_vtxPosAttribIndex = posIndex;

        vertex[posIndex]     = clip.x;
        vertex[posIndex + 1] = clip.y;
        vertex[posIndex + 2] = clip.z;
        vertex[posIndex + 3] = clip.w;

        vertID++;
    }
}