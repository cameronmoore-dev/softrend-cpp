#include "Image.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>

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

Image::~Image()
{
    delete[] m_pixels;
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

    m_pixels = new TGAPixel[pixelCount];

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

void Image::readUnCompressed(std::ifstream& file, bool hasAlphaChannel)
{
    for (size_t i = 0; i < m_width * m_height; ++i)
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