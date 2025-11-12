#include "Image.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

/* 0 = repeated, 1 = mirrored repeat, 2 = clamp to edge, 3 = clamp to border */
#define WRAP_MODE 0

Image::Image(const std::string& dir)
{
    std::filesystem::path path = std::filesystem::current_path().string() + dir;

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        std::cerr << "ERROR::IMAGE::failed to load image from dir: \n" << dir << std::endl;
        return;
    }
    
    loadImage(file);
}

void Image::loadImage(std::ifstream& file)
{
    TGAHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(TGAHeader));

    if (!file.good())
    {
        std::cerr << "ERROR::IMAGE::Header read failed" << std::endl;
        return;
    }

    m_width = header.width;
    m_height = header.height;
    m_bytesPerPixel= header.bitsPerPixel / 8;
    
    const size_t pixelCount = m_width * m_height;

    if (m_bytesPerPixel != 3 && m_bytesPerPixel != 4)
    {
        std::cerr << "ERROR::IMAGE::Unsupported bits per pixel: " << (int)m_bytesPerPixel*8 << std::endl;
        return;
    }

    m_pixels.resize(pixelCount);

    const bool isRLE = header.dataTypeCode == 10 || header.dataTypeCode == 11;
    const bool hasAlpha = m_bytesPerPixel == 4;
    const bool topLeftOrigin = (header.imageDescriptor & 0x20) != 0;

    if (!isRLE)
    {
        readUnCompressed(file, hasAlpha);
    } 
    else
    {
        readCompressed(file, pixelCount, hasAlpha);
    }
    /* Flip vertically if origin is top-left */
    if (topLeftOrigin)
    {
        for (size_t y = 0; y < m_height / 2; y++)
        {
            for (size_t x = 0; x < m_width; y++)
            {
                size_t topIdx = x + y * m_width;
                size_t botIdx = x + (m_height - 1 - y) * m_width;
                std::swap(m_pixels[topIdx], m_pixels[botIdx]);
            }
        }
    }

    file.close();

    MipMapLevel baseMip;
    baseMip.width = header.width;
    baseMip.height = header.height;
    baseMip.pixels = m_pixels;
    generateMipMaps(baseMip);
}

const float Image::srgbToLinear(const uint8_t component)
{
    float c = component / 255.0f; // Normalize to [0, 1]
    return std::powf(c, 2.2f);
}

void Image::write(const std::string& fileName)
{
    std::ofstream file(fileName, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "ERROR::IMAGE::Failed to open file for writing: " << fileName << std::endl;
        return;
    }

    TGAHeader header;
    header.dataTypeCode = 2;        /* Uncompressed true-color image */
    header.width = m_width;
    header.height = m_height;
    header.bitsPerPixel = 32;       /* Output 32 bits per pixel */
    header.imageDescriptor = 0x00;  /* Top-left origin */

    file.write(reinterpret_cast<char*>(&header), sizeof(header));

    for (size_t i = 0; i < m_width * m_height; i++)
    {
        const TGAPixel& pixel = m_pixels[i];

        uint8_t bgra[4] = { pixel.data[2], pixel.data[1], pixel.data[0], pixel.data[3] };

        file.write(reinterpret_cast<char*>(bgra), 4);
    }

    file.close();
}

const TGAPixel Image::bilinear(float u, float v, const MipMapLevel& mip) const
{
    u = std::clamp(u, 0.f, 1.f);
    v = std::clamp(v, 0.f, 1.f);

    float texX = u * (mip.width - 1);
    float texY = v * (mip.height - 1);

    int x = uint32_t(texX);
    int y = uint32_t(texY);

    float fx = texX - x;
    float fy = texY - y;

    int x1 = std::clamp(x, 0, mip.width - 1);
    int y1 = std::clamp(y, 0, mip.height - 1);
    int x2 = std::clamp(x + 1, 0, mip.width - 1);
    int y2 = std::clamp(y + 1, 0, mip.height - 1);

    const TGAPixel& tl = mip.pixels[x1 + y1 * mip.width];
    const TGAPixel& tr = mip.pixels[x2 + y1 * mip.width];
    const TGAPixel& bl = mip.pixels[x1 + y2 * mip.width];
    const TGAPixel& br = mip.pixels[x2 + y2 * mip.width];

    auto lerp = [](uint8_t a, uint8_t b, float t)
    {
        return (uint8_t)((1.0f - t) * a + t * b + 0.5f);
    };

    TGAPixel result;

    for (int i = 0; i < 4; i++)
    {
        uint8_t top = lerp(tl.data[i], tr.data[i], fx);
        uint8_t bottom = lerp(bl.data[i], br.data[i], fx);
        result.data[i] = lerp(top, bottom, fy);
    }

    return result;
}

const Vec3f Image::pixel(float u, float v, float lod)
{
    uint8_t level = std::clamp((int)(lod + 0.5f), 0, (int)m_mipLevels.size() - 1);
    currentMip = level;
    const MipMapLevel& mip = m_mipLevels[level];

    TGAPixel result = bilinear(u, v, mip);

    float r = srgbToLinear(result.data[0]);
    float g = srgbToLinear(result.data[1]);
    float b = srgbToLinear(result.data[2]);

    return { r, g, b };
}

const Vec4i Image::pixelRGBA(float u, float v, float lod)
{
    // uint8_t level = std::clamp((int)(lod + 0.5f), 0, (int)m_mipLevels.size() - 1);
    // currentMip = level;
    // const MipMapLevel& mip = m_mipLevels[level];

    // TGAPixel result = bilinear(u, v, mip);

    /* Shrinks texture size when projected onto object when multiplied and increases when divided */
    // u *= 3.f;
    // v *= 3.f;

    float uFraction = u - (long)u;
    float vFraction = v - (long)v;
    uint32_t x = uint32_t(uFraction * m_width)  % m_width;
    uint32_t y = uint32_t(vFraction * m_height) % m_height;

    if (WRAP_MODE == 1)
    {
        if ((int)std::ceil(u) & 1 != 0)
        {
            x = uint32_t((1.f - uFraction) * m_width)  % m_width;
        }

        if ((int)std::ceil(v) & 1 != 0)
        {
            y = uint32_t((1.f - vFraction) * m_height) % m_height;
        }
    }

    else if (WRAP_MODE == 2)
    {
        if (u > 1.f || u < 0.f)
        {
            x = (int)u * m_width;
        }

        if (v > 1.f || v < 0.f)
        {
            y = (int)v * m_height;
        }
    }

    else if (WRAP_MODE == 3)
    {
        if (u < 0.f || u > 1.f || 
            v < 0.f || v > 1.f)
        {
            return { (0xFF), (0x00), (0xFF), (0xFF) };
        }
    }
    
    TGAPixel texel = m_pixels[x + y * m_width];

    return { texel.data[0], texel.data[1], texel.data[2], texel.data[3] };
}

void Image::generateMipMaps(const MipMapLevel& base)
{
    m_mipLevels.emplace_back(base);
    MipMapLevel current = base;

    while (current.width > 1 && current.height > 1)
    {
        uint16_t newWidth = std::max(1, current.width / 2);
        uint16_t newHeight = std::max(1, current.height / 2);

        MipMapLevel next;
        next.width = newWidth;
        next.height = newHeight;
        next.pixels.resize(newWidth*newHeight);

        for (uint16_t y = 0; y < newHeight; y++)
        {
            for (uint16_t x = 0; x < newWidth; x++)
            {
                TGAPixel a = current.pixels[(x*2) + (y*2) * current.width];
                TGAPixel b = current.pixels[(x*2+1) + (y*2) * current.width];
                TGAPixel c = current.pixels[(x*2) + (y*2+1) * current.width];
                TGAPixel d = current.pixels[(x*2+1) + (y*2+1) * current.width];
                next.pixels[x + y * newWidth] = blendColours({ a, b, c, d });
            }
        }

        m_mipLevels.emplace_back(next);
        current = std::move(next);
    }
}

TGAPixel Image::blendColours(const std::initializer_list<TGAPixel>& colours)
{
    uint32_t r = 0;
    uint32_t g = 0;
    uint32_t b = 0;
    uint32_t a = 0;

    for (TGAPixel c : colours)
    {
        b += c.data[0];
        g += c.data[1];
        r += c.data[2];
        a += c.data[3];
    }

    uint8_t count = colours.size();
    TGAPixel blended;
    blended.data[0] = b / count;
    blended.data[1] = g / count;
    blended.data[2] = r / count;
    blended.data[3] = a / count;
    return blended;
}

float Image::lod(float dudx, float dudy, float dvdx, float dvdy)
{
    float dx = dudx * dudx + dvdx * dvdx;
    float dy = dudy * dudy + dvdy * dvdy;
    float rho = sqrt(std::max(dx, dy));
    float lod = std::log2(std::max(rho, 1e-5f));

    return std::max(0.0f, lod);
}

void Image::derivatives(const glTriangle& polygon, float& dudx, float& dudy, float& dvdx, float& dvdy)
{
    const float texWidth = m_mipLevels[0].width;
    const float texHeight = m_mipLevels[0].height;

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

void Image::readUnCompressed(std::ifstream& file, bool hasAlphaChannel)
{
    for (size_t i = 0; i < m_width * m_height; i++)
    {
        uint8_t imgPixel[4] = {0};
        file.read(reinterpret_cast<char*>(imgPixel), m_bytesPerPixel);

        TGAPixel& bufPixel = m_pixels[i];
        bufPixel.data[0] = imgPixel[2];
        bufPixel.data[1] = imgPixel[1];
        bufPixel.data[2] = imgPixel[0];
        bufPixel.data[3] = hasAlphaChannel ? imgPixel[3] : 255;
    }
}

void Image::readCompressed(std::ifstream& file, const size_t& pixelCount, bool hasAlphaChannel)
{
    size_t currentPixel = 0;
    while (currentPixel < pixelCount)
    {
        uint8_t chunkHeader = file.get();
        if (!file.good())
        {
            std::cerr << "ERROR::IMAGE::RLE chunk header read failed" << std::endl;
            return;
        }

        if (chunkHeader < 128)
        {
            chunkHeader++;
            readRawPacket(file, chunkHeader, currentPixel, hasAlphaChannel);
        } 
        else
        {
            chunkHeader -= 127;
            readRLEPacket(file, chunkHeader, currentPixel, hasAlphaChannel);
        }
    }
}

void Image::readRawPacket(std::ifstream& file, const uint8_t chunkHeader, size_t& currentPixel, bool hasAlphaChannel)
{
    for (int32_t i = 0; i < chunkHeader; i++)
    {
        uint8_t imgPixel[4] = {0};

        file.read(reinterpret_cast<char*>(imgPixel), m_bytesPerPixel);
        if (!file.good())
        {
            std::cerr << "ERROR::IMAGE::RLE run packet read failed" << std::endl;
            return;
        }

        TGAPixel& bufPixel = m_pixels[currentPixel++];
        bufPixel.data[0] = imgPixel[2];
        bufPixel.data[1] = imgPixel[1];
        bufPixel.data[2] = imgPixel[0];
        bufPixel.data[3] = hasAlphaChannel ? imgPixel[3] : 255;
    }
}

void Image::readRLEPacket(std::ifstream& file, const uint8_t chunkHeader, size_t& currentPixel, bool hasAlphaChannel)
{
    uint8_t imgPixel[4] = {0};

    file.read(reinterpret_cast<char*>(imgPixel), m_bytesPerPixel);
    if (!file.good())
    {
        std::cerr << "ERROR::IMAGE::RLE run packet read failed" << std::endl;
        return;
    }

    for (int32_t i = 0; i < chunkHeader; i++)
    {
        TGAPixel& bufPixel = m_pixels[currentPixel++];
        bufPixel.data[0] = imgPixel[2];
        bufPixel.data[1] = imgPixel[1];
        bufPixel.data[2] = imgPixel[0];
        bufPixel.data[3] = hasAlphaChannel ? imgPixel[3] : 255;
    }
}

void Image::fromDepthMap(const std::vector<float>& depthMap, int width, int height)
{
    m_width = width;
    m_height = height;
    m_pixels.resize(width*height);

    // Find max and min to normalize (optional, depends on your map range)
    float minDepth = 1.0f, maxDepth = 0.0f;
    for (float d : depthMap)
    {
        if (d < minDepth) minDepth = d;
        if (d > maxDepth) maxDepth = d;
    }

    // Avoid divide by zero
    float range = std::max(maxDepth - minDepth, 1e-5f);

    for (int i = 0; i < width * height; ++i)
    {
        float d = depthMap[i];

        // Normalize to [0, 1]
        float norm = (d - minDepth) / range;
        
        // Invert for visualization: closer = white
        uint8_t grayscale = static_cast<uint8_t>((1.0f - norm) * 255.0f);
        TGAPixel pixel = { grayscale, grayscale, grayscale, 255 };
        m_pixels[i] = pixel;
    }
}