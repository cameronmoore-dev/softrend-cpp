// #include "Image.h"

// #include <filesystem>
// #include <fstream>
// #include <iostream>
// #include <cstring>
// #include <cmath>
// #include <algorithm>

// /* 0 = repeated, 1 = mirrored repeat, 2 = clamp to edge, 3 = clamp to border */
// #define WRAP_MODE 0

// Image::Image(const std::string& dir)
// {
//     std::filesystem::path path = std::filesystem::current_path().string() + dir;

//     std::ifstream file(path, std::ios::binary);
//     if (!file)
//     {
//         std::cerr << "ERROR::IMAGE::failed to load image from dir: \n" << dir << std::endl;
//         return;
//     }
    
//     loadImage(file);
// }

// void Image::loadImage(std::ifstream& file)
// {
//     TGAHeader header;
//     file.read(reinterpret_cast<char*>(&header), sizeof(TGAHeader));

//     if (!file.good())
//     {
//         std::cerr << "ERROR::IMAGE::Header read failed" << std::endl;
//         return;
//     }

//     m_width = header.width;
//     m_height = header.height;
//     m_bytesPerPixel= header.bitsPerPixel / 8;
    
//     const size_t pixelCount = m_width * m_height;

//     if (m_bytesPerPixel != 3 && m_bytesPerPixel != 4)
//     {
//         std::cerr << "ERROR::IMAGE::Unsupported bits per pixel: " << (int)m_bytesPerPixel*8 << std::endl;
//         return;
//     }

//     m_pixels.resize(pixelCount);

//     const bool isRLE = header.dataTypeCode == 10 || header.dataTypeCode == 11;
//     const bool hasAlpha = m_bytesPerPixel == 4;
//     const bool topLeftOrigin = (header.imageDescriptor & 0x20) != 0;

//     if (!isRLE)
//     {
//         readUnCompressed(file, hasAlpha);
//     } 
//     else
//     {
//         readCompressed(file, pixelCount, hasAlpha);
//     }
//     /* Flip vertically if origin is top-left */
//     if (topLeftOrigin)
//     {
//         for (size_t y = 0; y < m_height / 2; y++)
//         {
//             for (size_t x = 0; x < m_width; y++)
//             {
//                 size_t topIdx = x + y * m_width;
//                 size_t botIdx = x + (m_height - 1 - y) * m_width;
//                 std::swap(m_pixels[topIdx], m_pixels[botIdx]);
//             }
//         }
//     }

//     file.close();

//     MipMapLevel baseMip;
//     baseMip.width = header.width;
//     baseMip.height = header.height;
//     baseMip.pixels = m_pixels;
//     generateMipMaps(baseMip);
// }

// const Vec4i Image::pixelRGBA(float u, float v, float lod)
// {
//     // uint8_t level = std::clamp((int)(lod + 0.5f), 0, (int)m_mipLevels.size() - 1);
//     // currentMip = level;
//     // const MipMapLevel& mip = m_mipLevels[level];

//     // TGAPixel result = bilinear(u, v, mip);

//     /* Shrinks texture size when projected onto object when multiplied and increases when divided */
//     // u *= 3.f;
//     // v *= 3.f;

//     float uFraction = u - (long)u;
//     float vFraction = v - (long)v;
//     uint32_t x = uint32_t(uFraction * m_width)  % m_width;
//     uint32_t y = uint32_t(vFraction * m_height) % m_height;

//     if (WRAP_MODE == 1)
//     {
//         if ((int)std::ceil(u) & 1 != 0)
//         {
//             x = uint32_t((1.f - uFraction) * m_width)  % m_width;
//         }

//         if ((int)std::ceil(v) & 1 != 0)
//         {
//             y = uint32_t((1.f - vFraction) * m_height) % m_height;
//         }
//     }

//     else if (WRAP_MODE == 2)
//     {
//         if (u > 1.f || u < 0.f)
//         {
//             x = (int)u * m_width;
//         }

//         if (v > 1.f || v < 0.f)
//         {
//             y = (int)v * m_height;
//         }
//     }

//     else if (WRAP_MODE == 3)
//     {
//         if (u < 0.f || u > 1.f || 
//             v < 0.f || v > 1.f)
//         {
//             return { (0xFF), (0x00), (0xFF), (0xFF) };
//         }
//     }
    
//     TGAPixel texel = m_pixels[x + y * m_width];

//     return { texel.data[0], texel.data[1], texel.data[2], texel.data[3] };
// }

// void Image::readUnCompressed(std::ifstream& file, bool hasAlphaChannel)
// {
//     for (size_t i = 0; i < m_width * m_height; i++)
//     {
//         uint8_t imgPixel[4] = {0};
//         file.read(reinterpret_cast<char*>(imgPixel), m_bytesPerPixel);

//         TGAPixel& bufPixel = m_pixels[i];
//         bufPixel.data[0] = imgPixel[2];
//         bufPixel.data[1] = imgPixel[1];
//         bufPixel.data[2] = imgPixel[0];
//         bufPixel.data[3] = hasAlphaChannel ? imgPixel[3] : 255;
//     }
// }

// void Image::readCompressed(std::ifstream& file, const size_t& pixelCount, bool hasAlphaChannel)
// {
//     size_t currentPixel = 0;
//     while (currentPixel < pixelCount)
//     {
//         uint8_t chunkHeader = file.get();
//         if (!file.good())
//         {
//             std::cerr << "ERROR::IMAGE::RLE chunk header read failed" << std::endl;
//             return;
//         }

//         if (chunkHeader < 128)
//         {
//             chunkHeader++;
//             readRawPacket(file, chunkHeader, currentPixel, hasAlphaChannel);
//         } 
//         else
//         {
//             chunkHeader -= 127;
//             readRLEPacket(file, chunkHeader, currentPixel, hasAlphaChannel);
//         }
//     }
// }

// void Image::readRawPacket(std::ifstream& file, const uint8_t chunkHeader, size_t& currentPixel, bool hasAlphaChannel)
// {
//     for (int32_t i = 0; i < chunkHeader; i++)
//     {
//         uint8_t imgPixel[4] = {0};

//         file.read(reinterpret_cast<char*>(imgPixel), m_bytesPerPixel);
//         if (!file.good())
//         {
//             std::cerr << "ERROR::IMAGE::RLE run packet read failed" << std::endl;
//             return;
//         }

//         TGAPixel& bufPixel = m_pixels[currentPixel++];
//         bufPixel.data[0] = imgPixel[2];
//         bufPixel.data[1] = imgPixel[1];
//         bufPixel.data[2] = imgPixel[0];
//         bufPixel.data[3] = hasAlphaChannel ? imgPixel[3] : 255;
//     }
// }

// void Image::readRLEPacket(std::ifstream& file, const uint8_t chunkHeader, size_t& currentPixel, bool hasAlphaChannel)
// {
//     uint8_t imgPixel[4] = {0};

//     file.read(reinterpret_cast<char*>(imgPixel), m_bytesPerPixel);
//     if (!file.good())
//     {
//         std::cerr << "ERROR::IMAGE::RLE run packet read failed" << std::endl;
//         return;
//     }

//     for (int32_t i = 0; i < chunkHeader; i++)
//     {
//         TGAPixel& bufPixel = m_pixels[currentPixel++];
//         bufPixel.data[0] = imgPixel[2];
//         bufPixel.data[1] = imgPixel[1];
//         bufPixel.data[2] = imgPixel[0];
//         bufPixel.data[3] = hasAlphaChannel ? imgPixel[3] : 255;
//     }
// }