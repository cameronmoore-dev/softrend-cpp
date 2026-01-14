#pragma once

#include <vector>
#include <algorithm>
#include <cstdint>

#include "Renderer.h"

#define QUALITY 3

#if QUALITY == 0
    #define EDGE_THRESHOLD (1.f/4.f)
    #define EDGE_THRESHOLD_MIN (1.f/16.f)

    #define SUBPIXEL 1
    #define SUBPIXEL_TRIM (1.f/4.f)
    #define SUBPIXEL_TRIM_SCALE (1.f/(1.f - SUBPIXEL_TRIM))
    #define SUBPIXEL_CAP (3.f/4.f)

    #define SEARCH_STEPS 8
    #define SEARCH_ACCELERATION 1
    #define SEARCH_THRESHOLD (1.f/4.f)

#elif QUALITY == 1
    #define EDGE_THRESHOLD (1.f/8.f)
    #define EDGE_THRESHOLD_MIN (1.f/16.f)

    #define SUBPIXEL 1
    #define SUBPIXEL_TRIM (1.f/4.f)
    #define SUBPIXEL_TRIM_SCALE (1.f/(1.f - SUBPIXEL_TRIM))
    #define SUBPIXEL_CAP (3.f/4.f)

    #define SEARCH_STEPS 12
    #define SEARCH_ACCELERATION 1
    #define SEARCH_THRESHOLD (1.f/4.f)

#elif QUALITY == 2
    #define EDGE_THRESHOLD (1.f/8.f)
    #define EDGE_THRESHOLD_MIN (1.f/16.f)

    #define SUBPIXEL 1
    #define SUBPIXEL_TRIM (1.f/4.f)
    #define SUBPIXEL_TRIM_SCALE (1.f/(1.f - SUBPIXEL_TRIM))
    #define SUBPIXEL_CAP (7.f/8.f)

    #define SEARCH_STEPS 24
    #define SEARCH_ACCELERATION 1
    #define SEARCH_THRESHOLD (1.f/4.f)

#elif QUALITY == 3
    #define EDGE_THRESHOLD (1.f/12.f)
    #define EDGE_THRESHOLD_MIN (1.f/16.f)

    #define SUBPIXEL 1
    #define SUBPIXEL_TRIM (1.f/8.f)
    #define SUBPIXEL_TRIM_SCALE (1.f/(1.f - SUBPIXEL_TRIM))
    #define SUBPIXEL_CAP (1.f)

    #define SEARCH_STEPS 32
    #define SEARCH_ACCELERATION 2
    #define SEARCH_THRESHOLD (1.f/8.f)

#endif

class FXAA_Shader
{
    uint32_t size = 0;
    uint16_t width = 0;

public:
    FXAA_Shader() = default;

    void apply(echo::RenderTarget &rendertarget)
    {
        size  = rendertarget.rect.width * rendertarget.rect.height;
        width = rendertarget.rect.width;
        for (uint32_t i = 0; i < size; i++)
        {
            /* Find contrast between pixel neighbours / Local Contrast Check */
            echo::vec3f rgbN = fxaaTextureOffset(rendertarget, i, -width);
            echo::vec3f rgbW = fxaaTextureOffset(rendertarget, i, -1);
            echo::vec3f rgbM = fxaaTextureOffset(rendertarget, i, 0);
            echo::vec3f rgbE = fxaaTextureOffset(rendertarget, i, 1);
            echo::vec3f rgbS = fxaaTextureOffset(rendertarget, i, width);

            float lumaN = fxaaLuma(rgbN);
            float lumaW = fxaaLuma(rgbW);
            float lumaM = fxaaLuma(rgbM);
            float lumaE = fxaaLuma(rgbE);
            float lumaS = fxaaLuma(rgbS);

            float rangeMin = std::min(lumaM, std::min(std::min(lumaN, lumaW), std::min(lumaS, lumaE)));
            float rangeMax = std::max(lumaM, std::max(std::max(lumaN, lumaW), std::max(lumaS, lumaE)));

            float range = rangeMax - rangeMin;

            if (range < std::max(EDGE_THRESHOLD_MIN, rangeMax * EDGE_THRESHOLD))
            {
                rendertarget.colour[i] = echo::rgbToHex(fxaaTextureOffset(rendertarget, i, 0));
                continue;
            }

            /* Low-pass filter blend / Sub-Pixel Aliasing Test */
            float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25f;
            float rangeL = std::abs(lumaL - lumaM);
            float blendL = std::max(0.f, (rangeL/range) - SUBPIXEL_TRIM) * SUBPIXEL_TRIM_SCALE;

            blendL = std::min(SUBPIXEL_CAP, blendL);

            /* Box filter around current pixel */
            echo::vec3f rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
            echo::vec3f rgbNW = fxaaTextureOffset(rendertarget, i, -width - 1);
            echo::vec3f rgbNE = fxaaTextureOffset(rendertarget, i, -width + 1);
            echo::vec3f rgbSW = fxaaTextureOffset(rendertarget, i, width - 1);
            echo::vec3f rgbSE = fxaaTextureOffset(rendertarget, i, width + 1);

            rgbL += rgbNW + rgbNE + rgbSW + rgbSE;
            rgbL *= echo::vec3f{ 1.f / 9.f, 0, 0 };

            float lumaNW = fxaaLuma(rgbNW);
            float lumaNE = fxaaLuma(rgbNE);
            float lumaSW = fxaaLuma(rgbSW);
            float lumaSE = fxaaLuma(rgbSE);

            /* Vertical/Horizontal Edge Test */
            float edgeVert = 
                std::abs((0.25f * lumaNW) + (-0.5f * lumaN) + (0.25f *  lumaNE)) +
                std::abs((0.5f * lumaW) + (-1.f * lumaM) + (0.5f * lumaE)) +
                std::abs((0.25f * lumaSW) + (-0.5f * lumaS) + (0.25f * lumaSE));

            float edgeHorz = 
                std::abs((0.25f * lumaNW) + (-0.5f * lumaW) + (0.25f *  lumaSW)) +
                std::abs((0.5f * lumaN) + (-1.f * lumaM) + (0.5f * lumaS)) +
                std::abs((0.25f * lumaNE) + (-0.5f * lumaE) + (0.25f * lumaSE));

            bool horzSpan = edgeHorz >= edgeVert;
            
            float threshold = range * SEARCH_THRESHOLD;
            float lumaNeg = lumaM;
            float lumaPos = lumaM;

            int offsetNeg = 0;
            int offsetPos = 0;

            for (uint32_t s = 0; s < SEARCH_STEPS; s++)
            {
                offsetNeg -= (horzSpan ? width : 1);
                offsetPos += (horzSpan ? width : 1);

                float lumaA = fxaaLuma(fxaaTextureOffset(rendertarget, i, offsetNeg));
                float lumaB = fxaaLuma(fxaaTextureOffset(rendertarget, i, offsetPos));

                if (lumaNeg != lumaM && lumaPos != lumaM)
                {
                    break;
                }

                if (std::abs(lumaA - lumaM) >= threshold && offsetNeg == 0)
                {
                    lumaNeg = lumaA;
                }

                if (std::abs(lumaB - lumaM) >= threshold && offsetPos == 0)
                {
                    lumaPos = lumaB;
                }
                
            }

            echo::vec3f rgbMVec = rgbM;
            echo::vec3f rgbLVec = fxaaTextureOffset(rendertarget, i, horzSpan ? (int)width : 1);
            echo::vec3f rgbRVec = fxaaTextureOffset(rendertarget, i, horzSpan ? -(int)width : -1);

            float dirLumaDiff = lumaNeg - lumaPos;
            echo::vec3f targetColour = dirLumaDiff > 0 ? rgbRVec : rgbLVec;
            echo::vec3f blended = rgbMVec * (1.f - blendL) + targetColour * blendL;
            
            rendertarget.colour[i] = echo::rgbToHex(blended);
        }
    }

private:
    float fxaaLuma(const echo::vec3f& colour)
    {
        return colour.y * (0.587f/0.299f) * colour.x;
        // return 0.299f * rgb.x + 0.587f * rgb.y + 0.114f * rgb.z;
    }

    echo::vec3f fxaaTextureOffset(echo::RenderTarget &rendertarget, int current, int offset)
    {
        int index = current + offset;
        if (index < 0) 
        {
            index = 0;
        }

        else if (index >= size)
        {
            index = size - 1;
        }

        uint32_t fragColour = rendertarget.colour[index];
        float r = float((fragColour >> 16) & 0xFF) / 255.f;
        float g = float((fragColour >> 8) & 0xFF) / 255.f;
        float b = float(fragColour & 0xFF) / 255.f;
        return  { r, g, b };
    }
};