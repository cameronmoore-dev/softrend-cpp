#pragma once

#include <string>
#include <stdint.h>

#include "Structs.h"

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

class Image
{
public:
    Image(){}
    Image(const std::string& dir);
    ~Image();

    const uint32_t pixel(uint32_t index) const
    {
        return (m_pixels[index].data[0] << 16) |
               (m_pixels[index].data[1] << 8)  | 
               (m_pixels[index].data[2])       |
               (m_pixels[index].data[3] << 24);
    }

    const Vec4i pixelRGBA(uint32_t index) const
    {
        return 
        {   
            m_pixels[index].data[0], 
            m_pixels[index].data[1], 
            m_pixels[index].data[2], 
            m_pixels[index].data[3]
        };
    }

    const Vec2i size() const { return { m_width, m_height }; }

    void write(const std::string& fileName);

    void fromDepthMap(const std::vector<float>& depthMap, int width, int height);

private:
    int16_t m_width = 0;
    int16_t m_height = 0;
    uint8_t m_bytesPerPixel = 0;
    TGAPixel* m_pixels;

private:
    void loadImage(std::ifstream& file);
    void readUnCompressed(std::ifstream& file, bool hasAlphaChannel);
    void readCompressed(std::ifstream& file, const size_t& pixelCount, bool hasAlphaChannel);
    void readRawPacket(std::ifstream& file, const uint8_t chunkHeader, size_t& currentPixel, bool hasAlphaChannel);
    void readRLEPacket(std::ifstream& file, const uint8_t chunkHeader, size_t& currentPixel, bool hasAlphaChannel);
};