#pragma once

#include <string>
#include <cstdint>

#include "vertex.h"

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

class Image
{
public:
    Image() = default;
    Image(const std::string& dir);

private:
    void loadImage(std::ifstream& file);
    void readUnCompressed(std::ifstream& file, bool hasAlphaChannel);
    void readCompressed(std::ifstream& file, const size_t& pixelCount, bool hasAlphaChannel);
    void readRawPacket(std::ifstream& file, const uint8_t chunkHeader, size_t& currentPixel, bool hasAlphaChannel);
    void readRLEPacket(std::ifstream& file, const uint8_t chunkHeader, size_t& currentPixel, bool hasAlphaChannel);
};