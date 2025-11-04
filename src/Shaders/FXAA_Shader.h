#pragma once

#include <vector>
#include <algorithm>

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
public:
    std::vector<uint32_t>* frameBuffer = nullptr;
    Vec2f frameSize;

    void apply()
    {
        for (uint32_t i = 0; i < frameBuffer->size(); i++)
        {
            /* Find contrast between pixel neighbours / Local Contrast Check */
            uint32_t rgbN = fxaaTextureOffset(i, -frameSize.x);
            uint32_t rgbW = fxaaTextureOffset(i, -1);
            uint32_t rgbM = fxaaTextureOffset(i, 0);
            uint32_t rgbE = fxaaTextureOffset(i, 1);
            uint32_t rgbS = fxaaTextureOffset(i, frameSize.x);

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
                frameBuffer->at(i) = fxaaTextureOffset(i, 0);
                continue;
            }

            /* Low-pass filter blend / Sub-Pixel Aliasing Test */
            float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25f;
            float rangeL = std::abs(lumaL - lumaM);
            float blendL = std::max(0.f, (rangeL/range) - SUBPIXEL_TRIM) * SUBPIXEL_TRIM_SCALE;

            blendL = std::min(SUBPIXEL_CAP, blendL);

            /* Box filter around current pixel */
            Vec3f rgbL = Utils::hexToRGB(rgbN) + Utils::hexToRGB(rgbW) + Utils::hexToRGB(rgbM) + Utils::hexToRGB(rgbE) + Utils::hexToRGB(rgbS);
            uint32_t rgbNW = fxaaTextureOffset(i, -frameSize.x - 1);
            uint32_t rgbNE = fxaaTextureOffset(i, -frameSize.x + 1);
            uint32_t rgbSW = fxaaTextureOffset(i, frameSize.x - 1);
            uint32_t rgbSE = fxaaTextureOffset(i, frameSize.x + 1);

            rgbL += (Utils::hexToRGB(rgbNW) + Utils::hexToRGB(rgbNE) + Utils::hexToRGB(rgbSW) + Utils::hexToRGB(rgbSE));
            rgbL *= Vec3f{ 1.f / 9.f, 0, 0 };

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
                offsetNeg -= (horzSpan ? frameSize.x : 1);
                offsetPos += (horzSpan ? frameSize.x : 1);

                float lumaA = fxaaLuma(fxaaTextureOffset(i, offsetNeg));
                float lumaB = fxaaLuma(fxaaTextureOffset(i, offsetPos));

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

            Vec3f rgbMVec = Utils::hexToRGB(rgbM);
            Vec3f rgbLVec =  Utils::hexToRGB(fxaaTextureOffset(i, horzSpan ? (int)frameSize.x : 1));
            Vec3f rgbRVec =  Utils::hexToRGB(fxaaTextureOffset(i, horzSpan ? -(int)frameSize.x : -1));

            float dirLumaDiff = lumaNeg - lumaPos;
            Vec3f targetColour = dirLumaDiff > 0 ? rgbRVec : rgbLVec;
            Vec3f blended = rgbMVec * (1.f - blendL) + targetColour * blendL;
            
            frameBuffer->at(i) = Utils::rgbToHex(blended);
        }
    }

private:
    float fxaaLuma(const uint32_t& colour)
    {
        Vec3f rgb = Utils::hexToRGB(colour);
        return rgb.y * (0.587f/0.299f) * rgb.x;
        // return 0.299f * rgb.x + 0.587f * rgb.y + 0.114f * rgb.z;
    }

    uint32_t fxaaTextureOffset(int current, int offset)
    {
        int index = current + offset;
        if (index < 0) 
        {
            index = 0;
        }

        else if (index >= frameBuffer->size())
        {
            index = (frameBuffer->size()) - 1;
        }

        return frameBuffer->at(index);
    }
};