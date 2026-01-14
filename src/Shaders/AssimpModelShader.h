#pragma once

#include "IShader.h"

class AssimpModelShader : public echo::IShader
{
public:
    Uniform<echo::mat4> model;
    Uniform<echo::mat4> view;
    Uniform<echo::mat4> projection;

    echo::mat4 lightSpaceMatrix;
    echo::vec3f lightDir;

    Uniform<echo::Texture> diffuse;
    std::vector<float> shadowMap;
    
    echo::vec3f ambient = { 0.17f, 0.17f, 0.17f };
    uint16_t shadowMapSize = 0;

    void vertex(float *vertex) override
    {
        echo::vec3f position = getVertexAttribData<echo::vec3f>(vertex, 0);

        echo::vec4f world = *model * vec4(position, 1.f);
        echo::vec4f clip = *projection * (*view * world);
        glPosition(0, vertex, clip);

        /* Fill the FragPos with world space coordinates */
        vertex[9]  = world.x;
        vertex[10] = world.y;
        vertex[11] = world.z;
    }

    bool fragment(const echo::vec3f& barycentric, const echo::Face &face, echo::vec4f& fragColour) override
    {
        std::array<echo::vec4f, 3> pos = getFaceAttrib<echo::vec4f>(face, 0);
        std::array<echo::vec3f, 3> fragWorld = getFaceAttrib<echo::vec3f>(face, 3);

        float invW0 = 1.f / pos[0].w;
        float invW1 = 1.f / pos[1].w;
        float invW2 = 1.f / pos[2].w;
        echo::vec3f pcf0 = fragWorld[0] / pos[0].w;
        echo::vec3f pcf1 = fragWorld[1] / pos[1].w;
        echo::vec3f pcf2 = fragWorld[2] / pos[2].w;

        float inverse = invW0 * barycentric.x + invW1 * barycentric.y + invW2 * barycentric.z;
        echo::vec3f worldFragPos = (pcf0 * barycentric.x + pcf1 * barycentric.y + pcf2 * barycentric.z) / inverse;

        float shadow = shadowCalc(worldFragPos);

        fragColour = vec4(ambient + (1.f - shadow), 1.f) * texture(2, face, *diffuse, barycentric);
        return true;
    }

    float shadowCalc(const echo::vec3f &worldFragPos)
    {
        echo::vec4f lightPos = lightSpaceMatrix * vec4(worldFragPos, 1.f);
        echo::vec3f proj = vec3(lightPos) / lightPos.w;
        lightDir = echo::normalise(lightDir);

        proj.x = proj.x * 0.5f + 0.5f;
        proj.y = proj.y * 0.5f + 0.5f;

        if (proj.x < 0.f || proj.x > 1.f ||
            proj.y < 0.f || proj.y > 1.f)
        {
            return 0.f;
        }

        int sx = int(proj.x * (shadowMapSize - 1));
        int sy = int(proj.y * (shadowMapSize - 1));

        float shdwdepth = shadowMap[sx + sy * shadowMapSize];
        return (proj.z - 0.03f) > shdwdepth ? 1.f : 0.f;
    }
};