#include "IShader.h"

#include <cmath>

Vec3f IShader::vec3(const Vec4f &vec)
{
    return { vec.x, vec.y, vec.z };
}

Vec4f IShader::vec4(const Vec3f &vec, const float w)
{
    return { vec.x, vec.y, vec.z, w };
}

Vec4f IShader::vec4(const Vec2f &vec, const float z, const float w)
{
    return { vec.x, vec.y, z, w };
}

float IShader::dot(const Vec3f& a, const Vec3f& b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float IShader::length(const Vec3f& normal)
{
    return sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
}

Vec3f IShader::cross(const Vec3f& a, const Vec3f& b)
{
    Vec3f c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;

    return c;
}

Vec3f IShader::reflect(const Vec3f& i, const Vec3f& n)
{
    return i - n * (2.0f * dot(n, i));
}

Vec3f IShader::normalise(const Vec3f& vec)
{
    float mag = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    return { vec.x / mag, vec.y / mag, vec.z / mag };
}

void IShader::derivatives(const int uvIndex, const face_t &face, Texture &texture, float& dudx, float& dudy, float& dvdx, float& dvdy)
{
    const float texWidth  = texture.mipLevels[0].width;
    const float texHeight = texture.mipLevels[0].height;

    int stride = _priv::vertexdef.stride;
    std::array<Vec4f, 3> positions = getFaceAttrib4(face, 0);

    float x1 = positions[1].x - positions[0].x;
    float y1 = positions[1].y - positions[0].y;
    float x2 = positions[2].x - positions[0].x;
    float y2 = positions[2].y - positions[0].y;

    Vec2f uv0 = getVertexAttribData2(&face.v0[0], uvIndex);
    Vec2f uv1 = getVertexAttribData2(&face.v1[0], uvIndex);
    Vec2f uv2 = getVertexAttribData2(&face.v2[0], uvIndex);

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

Vec2f IShader::perspectiveCorrectedUV(const int uvIndex, const face_t &face, const Vec3f& barycentric)
{
    /* Perspective-correct interpolation */
    // float invInterp = inverseWs[vertIndex - 3] * barycentric.x +
    //                   inverseWs[vertIndex - 2] * barycentric.y +
    //                   inverseWs[vertIndex - 1] * barycentric.z;

    Vec2f uv0 = getVertexAttribData2(&face.v0[0], uvIndex);
    Vec2f uv1 = getVertexAttribData2(&face.v1[0], uvIndex);
    Vec2f uv2 = getVertexAttribData2(&face.v2[0], uvIndex);
    
    Vec2f finalUV = uv0 * barycentric.x +
                    uv1 * barycentric.y +
                    uv2 * barycentric.z;

    return { finalUV.x, finalUV.y };
}

const float IShader::srgbToLinear(const uint8_t channel)
{
    float c = channel / 255.0f; // Normalize to [0, 1]
    return std::powf(c, 2.2f);
}

const uint32_t IShader::bilinear(float u, float v, const MipMap &mip) const
{
    auto byte = [](const uint32_t colour, int shift)
    {
        return static_cast<uint8_t>((colour >> shift) & 0xFF);
    };

    auto lerp = [](const uint8_t a, const uint8_t b, float t)
    {
        return (uint8_t)((1.0f - t) * a + t * b + 0.5f);
    };

    u = std::clamp(u, 0.f, 1.f);
    v = std::clamp(v, 0.f, 1.f);

    float tx = u * (mip.width - 1);
    float ty = v * (mip.height - 1);
    int x = (int)tx;
    int y = (int)ty;
    float fx = tx - x;
    float fy = ty - y;

    int x1 = std::clamp(x, 0, mip.width  - 1);
    int y1 = std::clamp(y, 0, mip.height - 1);
    int x2 = std::clamp(x + 1, 0, mip.width  - 1);
    int y2 = std::clamp(y + 1, 0, mip.height - 1);

    const uint32_t &tl = mip.data[x1 + y1 * mip.width];
    const uint32_t &tr = mip.data[x2 + y1 * mip.width];
    const uint32_t &bl = mip.data[x1 + y2 * mip.width];
    const uint32_t &br = mip.data[x2 + y2 * mip.width];

    uint8_t topleft[4]     = { byte(tl, 16), byte(tl, 8), byte(tl, 0), byte(tl, 24) };
    uint8_t topright[4]    = { byte(tr, 16), byte(tr, 8), byte(tr, 0), byte(tr, 24) };
    uint8_t bottomleft[4]  = { byte(bl, 16), byte(bl, 8), byte(bl, 0), byte(bl, 24) };
    uint8_t bottomright[4] = { byte(br, 16), byte(br, 8), byte(br, 0), byte(br, 24) };

    uint8_t result[4] = {};
    for (int i = 0; i < 4; i++)
    {
        uint8_t top     = lerp(topleft[i], topright[i], fx);
        uint8_t bottom  = lerp(bottomleft[i], bottomright[i], fx);
        result[i]       = lerp(top, bottom, fy);
    }

    return ((result[0] << 16) | (result[1] << 8) | (result[2] << 0) | (result[3] << 24));
}

Vec4f IShader::pixelRGBA(const Texture &texture, float u, float v, float lod)
{
    uint8_t level = std::clamp((int)(lod + 0.5f), 0, (int)texture.mipLevels.size() - 1);
    const MipMap &mip = texture.mipLevels[level];

    uint32_t result = bilinear(u, v, mip);

    // int x = u * (mip.width - 1);
    // int y = v * (mip.height - 1);
    // uint32_t result = mip.data[x + y * mip.width];

    float r = srgbToLinear((result >> 16) & 0xFF);
    float g = srgbToLinear((result >> 8) & 0xFF);
    float b = srgbToLinear(result & 0xFF);
    float a = srgbToLinear((result >> 24) & 0xFF);

    // float r = ((result >> 16) & 0xFF) / 255.f;
    // float g = ((result >> 8)  & 0xFF) / 255.f;
    // float b = ((result >> 0)  & 0xFF) / 255.f;
    // float a = ((result >> 24) & 0xFF) / 255.f;

    return { r, g, b, a };
}

Vec4f IShader::pixelRGBA(const Texture &texture, float u, float v)
{
    // uint8_t level = std::clamp((int)(lod + 0.5f), 0, (int)texture.mipLevels.size() - 1);
    // const MipMap &mip = texture.mipLevels[level];

    // uint32_t result = bilinear(u, v, mip);

    int x = u * (texture.mipLevels[0].width - 1);
    int y = v * (texture.mipLevels[0].height - 1);

    uint32_t result = texture.mipLevels[0].data[x + y * texture.mipLevels[0].width];

    float r = srgbToLinear((result >> 16) & 0xFF);
    float g = srgbToLinear((result >> 8) & 0xFF);
    float b = srgbToLinear(result & 0xFF);
    float a = srgbToLinear((result >> 24) & 0xFF);

    // float r = ((result >> 16) & 0xFF) / 255.f;
    // float g = ((result >> 8)  & 0xFF) / 255.f;
    // float b = ((result >> 0)  & 0xFF) / 255.f;
    // float a = ((result >> 24) & 0xFF) / 255.f;

    return { r, g, b, a };
}

Vec3f IShader::pixelRGB(const Texture &texture, float u, float v, float lod)
{
    uint8_t level = std::clamp((int)(lod + 0.5f), 0, (int)texture.mipLevels.size() - 1);
    const MipMap &mip = texture.mipLevels[level];

    // uint32_t result = bilinear(u, v, mip);

    int x = u * (mip.width - 1);
    int y = v * (mip.height - 1);
    uint32_t result = mip.data[x + y * mip.width];

    float r = ((result >> 16) & 0xFF) / 255.f;
    float g = ((result >> 8)  & 0xFF) / 255.f;
    float b = ((result >> 0)  & 0xFF) / 255.f;

    return { r, g, b };
}

Vec3f IShader::pixelRGB(const Texture &texture, float u, float v)
{
    // uint8_t level = std::clamp((int)(lod + 0.5f), 0, (int)texture.mipLevels.size() - 1);
    // const MipMap &mip = texture.mipLevels[level];

    // uint32_t result = bilinear(u, v, mip);

    const MipMap &mip = texture.mipLevels[0];

    int x = u * (mip.width - 1);
    int y = v * (mip.height - 1);
    uint32_t result = mip.data[x + y * mip.width];

    float r = ((result >> 16) & 0xFF) / 255.f;
    float g = ((result >> 8)  & 0xFF) / 255.f;
    float b = ((result >> 0)  & 0xFF) / 255.f;

    return { r, g, b };
}

Vec4f IShader::sampleDiffuseMap(const int uvIndex, const face_t &face, Texture &texture, const Vec3f& barycentric)
{
    float dudx, dudy, dvdx, dvdy;
    derivatives(uvIndex, face, texture, dudx, dudy, dvdx, dvdy);
    float _lod = lod(dudx, dudy, dvdx, dvdy);

    Vec2f uv = perspectiveCorrectedUV(uvIndex, face, barycentric);
    return pixelRGBA(texture, uv.x, uv.y, _lod);
}

Vec3f IShader::sampleNormalMap(const face_t &face, Texture &texture, const Vec3f& barycentric)
{
    int stride = getVertexStride();
    std::array<Vec3f, 3> normals = getFaceAttrib(face, 1);
    std::array<Vec3f, 3> tangents = getFaceAttrib(face, 3);
    std::array<Vec3f, 3> bitangents = getFaceAttrib(face, 4);

    // Perspective-correct TBN
    Vec3f T = tangents[0] * barycentric.x + tangents[1] * barycentric.y + tangents[2] * barycentric.z;
    Vec3f B = bitangents[0] * barycentric.x + bitangents[1] * barycentric.y + bitangents[2] * barycentric.z;
    Vec3f N = normals[0] * barycentric.x + normals[1] * barycentric.y + normals[2] * barycentric.z;

    float dudx, dudy, dvdx, dvdy;
    derivatives(2, face, texture, dudx, dudy, dvdx, dvdy);
    float _lod = lod(dudx, dudy, dvdx, dvdy);

    Vec2f uv = perspectiveCorrectedUV(2, face, barycentric);
    Vec3f texel = pixelRGB(texture, uv.x, uv.y, _lod);
    Vec3f normal = normalise(texel * 2.f - 1.f);
    Mat3 tbn = Matrix::constructTBN(T, B, N);

    return normalise(Matrix::multiplyVec3(tbn, normal));
}

Vec3f IShader::sampleSpecularMap(const face_t &face, Texture &texture, const Vec3f& barycentric)
{
    // float dudx, dudy, dvdx, dvdy;
    // derivatives(polygon, texture, dudx, dudy, dvdx, dvdy);

    // float _lod = lod(dudx, dudy, dvdx, dvdy);

    Vec2f uv = perspectiveCorrectedUV(2, face, barycentric);
    return pixelRGB(texture, uv.x, uv.y);
}