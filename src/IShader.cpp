#include "IShader.h"

#include <cmath>

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

void IShader::derivatives(const glTriangle& polygon, Texture &texture, float& dudx, float& dudy, float& dvdx, float& dvdy)
{
    const float texWidth = texture.mipLevels[0].width;
    const float texHeight = texture.mipLevels[0].height;

    const auto& p0 = polygon.v0.position;
    const auto& p1 = polygon.v1.position;
    const auto& p2 = polygon.v2.position;

    float x1 = p1.x - p0.x;
    float y1 = p1.y - p0.y;
    float x2 = p2.x - p0.x;
    float y2 = p2.y - p0.y;

    Vec2f uv0 = polygon.v0.orig_uv;
    Vec2f uv1 = polygon.v1.orig_uv;
    Vec2f uv2 = polygon.v2.orig_uv;

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
    float rho = sqrt(std::max(dx, dy));
    float lod = std::log2(std::max(rho, 1e-5f));

    return std::max(0.0f, lod);
}

Vec2f IShader::perspectiveCorrectedUV(const glTriangle& polygon, const Vec3f& barycentric)
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

Vec3f IShader::pixelRGB(const Texture &texture, float u, float v, float lod)
{
    uint8_t level = std::clamp((int)(lod + 0.5f), 0, (int)texture.mipLevels.size() - 1);
    const MipMap &mip = texture.mipLevels[level];

    uint32_t result = bilinear(u, v, mip);

    // int x = u * (mip.width - 1);
    // int y = v * (mip.height - 1);
    // uint32_t result = mip.data[x + y * mip.width];

    float r = ((result >> 16) & 0xFF) / 255.f;
    float g = ((result >> 8)  & 0xFF) / 255.f;
    float b = ((result >> 0)  & 0xFF) / 255.f;

    return { r, g, b };
}

Vec4f IShader::sampleDiffuseMap(const glTriangle& polygon, Texture &texture, const Vec3f& barycentric)
{
    float dudx, dudy, dvdx, dvdy;
    derivatives(polygon, texture, dudx, dudy, dvdx, dvdy);

    Vec2f uv = perspectiveCorrectedUV(polygon, barycentric);

    return pixelRGBA(texture, uv.x, uv.y, lod(dudx, dudy, dvdx, dvdy));
}

Vec3f IShader::sampleNormalMap(const glTriangle& polygon, Texture &texture, const Vec3f& barycentric)
{
    // Perspective-correct TBN
    Vec3f T = polygon.v0.tangent * barycentric.x + polygon.v1.tangent * barycentric.y + polygon.v2.tangent * barycentric.z;
    Vec3f B = polygon.v0.bitangent * barycentric.x + polygon.v1.bitangent * barycentric.y + polygon.v2.bitangent * barycentric.z;
    Vec3f N = polygon.v0.normal * barycentric.x + polygon.v1.normal * barycentric.y + polygon.v2.normal * barycentric.z;

    float dudx, dudy, dvdx, dvdy;
    derivatives(polygon, texture, dudx, dudy, dvdx, dvdy);
    float _lod = lod(dudx, dudy, dvdx, dvdy);

    Vec2f uv = perspectiveCorrectedUV(polygon, barycentric);
    Vec3f texel = pixelRGB(texture, uv.x, uv.y, _lod);
    Vec3f normal;
    normal.x = texel.x * 2.f - 1.f;
    normal.y = texel.y * 2.f - 1.f;
    normal.z = texel.z * 2.f - 1.f;
    normal = normalise(normal);

    Mat3 tbn = Matrix::constructTBN(T, B, N);

    return normalise(Matrix::multiplyVec3(tbn, normal));
}

Vec3f IShader::sampleSpecularMap(const glTriangle& polygon, Texture &texture, const Vec3f& barycentric)
{
    float dudx, dudy, dvdx, dvdy;
    derivatives(polygon, texture, dudx, dudy, dvdx, dvdy);

    float _lod = lod(dudx, dudy, dvdx, dvdy);

    Vec2f uv = perspectiveCorrectedUV(polygon, barycentric);
    return pixelRGB(texture, uv.x, uv.y, _lod);
}