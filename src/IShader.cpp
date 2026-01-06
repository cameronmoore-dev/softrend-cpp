#include "IShader.h"

#include <cmath>

namespace echo
{
    void IShader::reset()
    {
        vertIndex = 0;
        fragIndex = 0;
    }

    vec3f IShader::reflect(const vec3f& i, const vec3f& n)
    {
        return i - n * (2.0f * dot(n, i));
    }
    
    void IShader::derivatives(const int uvIndex, const face &face, const Texture &texture, float& dudx, float& dudy, float& dvdx, float& dvdy)
    {
        const float texWidth  = texture.mipLevels[0].width;
        const float texHeight = texture.mipLevels[0].height;
    
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
    
    vec2f IShader::perspectiveCorrectedUV(const int uvIndex, const face &face, const vec3f& barycentric)
    {
        /* Perspective-correct interpolation */
        // float invInterp = inverseWs[vertIndex - 3] * barycentric.x +
        //                   inverseWs[vertIndex - 2] * barycentric.y +
        //                   inverseWs[vertIndex - 1] * barycentric.z;
    
        vec2f uv0 = getVertexAttribData<vec2f>(&face.v0[0], uvIndex);
        vec2f uv1 = getVertexAttribData<vec2f>(&face.v1[0], uvIndex);
        vec2f uv2 = getVertexAttribData<vec2f>(&face.v2[0], uvIndex);
        
        vec2f finalUV = uv0 * barycentric.x +
                        uv1 * barycentric.y +
                        uv2 * barycentric.z;
    
        return { finalUV.x, finalUV.y };
    }
    
    vec4f IShader::texture(const int uvIndex, const face &face, Texture &texture, const vec3f& barycentric)
    {
        float dudx, dudy, dvdx, dvdy;
        derivatives(uvIndex, face, texture, dudx, dudy, dvdx, dvdy);
        float _lod = lod(dudx, dudy, dvdx, dvdy);
    
        vec2f uv = perspectiveCorrectedUV(uvIndex, face, barycentric);

        uint8_t level = std::clamp((int)(_lod + 0.5f), 0, (int)texture.mipLevels.size() - 1);
        const MipMap &mip = texture.mipLevels[level];
    
        int x = uv.x * (mip.width - 1);
        int y = uv.y * (mip.height - 1);
        uint32_t result = mip.data[x + y * mip.width];
    
        vec4f rgba = 
        {
            ((result >> 16) & 0xFF) / 255.f,
            ((result >> 8)  & 0xFF) / 255.f,
            ((result >> 0)  & 0xFF) / 255.f,
            ((result >> 24) & 0xFF) / 255.f
        };
    
        return rgba;
    }

    void IShader::glPosition(int posIndex, float *position, const vec4f &clip)
    {
        position[posIndex]     = clip.x;
        position[posIndex + 1] = clip.y;
        position[posIndex + 2] = clip.z;
        position[posIndex + 3] = clip.w;
        vertIndex++;
    }
}