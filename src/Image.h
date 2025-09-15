#pragma once

#include <string>
#include <stdint.h>

#include "Structs.h"
#include "Utils.h"

#pragma pack(push, 1)
struct TGAHeader
{
    uint8_t idLength = 0;
    uint8_t colourMap = 0;
    uint8_t dataTypeCode = 0;
    uint16_t colourMapOrigin = 0;
    uint16_t colourMapLength = 0;
    uint8_t colourMapDepth = 0;
    uint16_t xOrigin = 0;
    uint16_t yOrigin = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t bitsPerPixel = 0;
    uint8_t imageDescriptor = 0;
};
#pragma pack(pop)

struct TGAPixel
{
    /* BGRA */
    uint8_t data[4] = {0, 0, 0, 0};
};

struct MipMapLevel
{
    uint16_t width;
    uint16_t height;
    std::vector<TGAPixel> pixels;
};

class Image
{
public:
    Image(){}
    Image(const std::string& dir);

    const Vec3f pixel(float u, float v, float lod);
    const Vec4i pixelRGBA(float u, float v, float lod);

    void derivatives(const glTriangle& polygon, float& dudx, float& dudy, float& dvdx, float& dvdy);
    float lod(float dudx, float dudy, float dvdx, float dvdy);

    const Vec2i size() const { return { m_width, m_height }; }

    void write(const std::string& fileName);

    void fromDepthMap(const std::vector<float>& depthMap, int width, int height);

    uint8_t currentMip;
    std::vector<MipMapLevel> m_mipLevels;

private:
    int16_t m_width = 0;
    int16_t m_height = 0;
    uint8_t m_bytesPerPixel = 0;
    std::vector<TGAPixel> m_pixels;

private:
    void loadImage(std::ifstream& file);
    void readUnCompressed(std::ifstream& file, bool hasAlphaChannel);
    void readCompressed(std::ifstream& file, const size_t& pixelCount, bool hasAlphaChannel);
    void readRawPacket(std::ifstream& file, const uint8_t chunkHeader, size_t& currentPixel, bool hasAlphaChannel);
    void readRLEPacket(std::ifstream& file, const uint8_t chunkHeader, size_t& currentPixel, bool hasAlphaChannel);

    void generateMipMaps(const MipMapLevel& base);
    TGAPixel blendColours(const std::initializer_list<TGAPixel>& colours);

    const TGAPixel bilinear(float u, float v, const MipMapLevel& mip) const;

    const float srgbToLinear(const uint8_t channel);
};