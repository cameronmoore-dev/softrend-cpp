#pragma once

#include <vector>
#include <algorithm>
#include <random>
#include <cmath>

#include "Renderer/renderer.h"

class SSAO_Shader
{
private:
    uint16_t width;
    uint16_t height;

    float aoRadius = 8.f;
    int samples = 32;

    std::vector<std::pair<float, float>> kernel;
    std::vector<float> aoBuffer;
    std::vector<float> blurBuffer;

public:
    bool doBlur = false;

public:
    SSAO_Shader()
    {
        initKernel();
    }

    void initAOBuffers(const uint16_t width, const uint16_t height)
    {
        aoBuffer.resize(width*height, 0.f);
        blurBuffer.resize(width*height, 0.f);
    }
    
    void apply(RenderTarget &rendertarget)
    {
        std::fill(aoBuffer.begin(), aoBuffer.end(), 0.f);
        std::fill(blurBuffer.begin(), blurBuffer.end(), 0.f);

        width  = rendertarget.rect.width;
        height = rendertarget.rect.height;

        auto smoothstep = [](float edge0, float edge1, float x)
        {
            float t = std::clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f);
            return t * t * (3.f - 2.f * t);
        };

        for (int i = 0; i < width * height; i++)
        {
            float z = rendertarget.depth[i];
            if (z > 1.0f)
            {
                continue;
            }

            float vote = 0.f;
            int voters = 0;

            int x = i % width;
            int y = i / width;

            for (const auto& [dx, dy] : kernel)
            {
                int sx = x + (int)dx;
                int sy = y + (int)dy;

                if (sx < 0 || sx >= width || sy < 0 || sy >= height)
                {
                    continue;
                }

                float sampleDepth = rendertarget.depth[sx + sy * width];
                float diff = sampleDepth - z;
                
                if (diff > 0.02f * z)
                {
                    continue;
                }

                vote += diff < 0.f;
                voters++;
            }

            if (voters == 0)
            {
                continue;
            }

            aoBuffer[i] = smoothstep(0.f, 1.f, 1.f - (vote / voters) * 0.5f);
        }

        if (doBlur)
        {
            bilateralBlur(rendertarget);
        }
        
        for (u32 i = 0; i < aoBuffer.size(); ++i)
        {
            float ao = aoBuffer[i];
            if (ao == 0.f)
            {
                continue;
            }

            vec3f fragColour = hexToRGB(rendertarget.colour[i]);
            fragColour.x *= ao;
            fragColour.y *= ao;
            fragColour.z *= ao;
            rendertarget.colour[i] = rgbToHex(fragColour);
        }
    }

    void bilateralBlur(const RenderTarget &rendertarget)
    {
        int radius = 4;          // Blur radius
        float sigmaSpace = 2.0f; // Spatial falloff
        float sigmaDepth = 50.f; // Depth sensitivity (adjust per scene scale)

        auto gaussian = [](float x, float sigma)
        {
            return expf(-(x * x) / (2.f * sigma * sigma));
        };

        // --- Horizontal pass ---
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int idx = x + y * width;

                float centerDepth = rendertarget.depth[idx];
                float centerAO = aoBuffer[idx];
                if (centerDepth > 1.0f || centerAO == 0.f)
                {
                    blurBuffer[idx] = centerAO;
                    continue;
                }

                float sum = 0.f;
                float weightSum = 0.f;

                for (int dx = -radius; dx <= radius; dx++)
                {
                    int sx = std::clamp(x + dx, 0, width - 1);
                    int sidx = sx + y * width;
                    float sampleAO = aoBuffer[sidx];
                    float sampleDepth = rendertarget.depth[sidx];

                    if (sampleAO == 0.f || sampleDepth > 1.0f)
                    {
                        continue;
                    }

                    float wSpace = gaussian((float)dx, sigmaSpace);
                    float depthDiff = fabs(centerDepth - sampleDepth);
                    float wDepth = gaussian(depthDiff * 100.f, sigmaDepth);

                    float weight = wSpace * wDepth;
                    sum += sampleAO * weight;
                    weightSum += weight;
                }

                blurBuffer[idx] = (weightSum > 0.f) ? (sum / weightSum) : centerAO;
            }
        }

        // --- Vertical pass ---
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int idx = x + y * width;

                float centerDepth = rendertarget.depth[idx];
                float centerAO = blurBuffer[idx];
                if (centerDepth > 1.0f || centerAO == 0.f)
                {
                    aoBuffer[idx] = centerAO;
                    continue;
                }

                float sum = 0.f;
                float weightSum = 0.f;

                for (int dy = -radius; dy <= radius; ++dy)
                {
                    int sy = std::clamp(y + dy, 0, height - 1);
                    int sidx = x + sy * width;

                    float sampleAO = blurBuffer[sidx];
                    float sampleDepth = rendertarget.depth[sidx];

                    if (sampleAO == 0.f || sampleDepth > 1.0f)
                    {
                        continue;
                    }

                    float wSpace = gaussian((float)dy, sigmaSpace);
                    float depthDiff = fabs(centerDepth - sampleDepth);
                    float wDepth = gaussian(depthDiff * 100.f, sigmaDepth);
                    float weight = wSpace * wDepth;

                    sum += sampleAO * weight;
                    weightSum += weight;
                }

                aoBuffer[idx] = (weightSum > 0.f) ? (sum / weightSum) : centerAO;
            }
        }
    }

private:
    void initKernel()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist { 0.f, 2.f * pi };
        std::uniform_real_distribution<float> radiusDist{ 0.f, 1.f };

        for (int i = 0; i < samples; i++)
        {
            float r = radiusDist(gen);
            float scale = (float)i / (float)samples;
            
            r = r * r * aoRadius * (0.1f + 0.9f * scale);

            float theta = angleDist(gen);
            float dx = r * cos(theta);
            float dy = r * sin(theta);

            kernel.emplace_back(dx, dy);
        }
    }
};