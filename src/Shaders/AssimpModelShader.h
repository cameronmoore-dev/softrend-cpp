#pragma once

#include "Renderer/shader.h"

class AssimpModelShader : public IShader
{
public:
    Uniform<mat4> model;
    Uniform<mat4> view;
    Uniform<mat4> projection;

    mat4 lightSpaceMatrix;
    vec3f lightDir;

    Uniform<Texture> diffuse;
    std::vector<float> shadowMap;
    
    vec3f ambient = { 0.17f, 0.17f, 0.17f };
    uint16_t shadowMapSize = 0;

    void vertex(float *vertex) override
    {
        vec3f position = getVertexAttribData<vec3f>(vertex, 0);

        vec4f world = *model * vec4(position, 1.f);
        vec4f clip = *projection * (*view * world);
        glPosition(0, vertex, clip);

        /* Fill the FragPos with world space coordinates */
        vertex[9]  = world.x;
        vertex[10] = world.y;
        vertex[11] = world.z;
    }

    bool fragment(const vec3f& barycentric, const Face &face, vec4f& fragColour) override
    {
        std::array<vec4f, 3> pos = getFaceAttrib<vec4f>(face, 0);
        std::array<vec3f, 3> fragWorld = getFaceAttrib<vec3f>(face, 3);

        float invW0 = 1.f / pos[0].w;
        float invW1 = 1.f / pos[1].w;
        float invW2 = 1.f / pos[2].w;
        vec3f pcf0 = fragWorld[0] / pos[0].w;
        vec3f pcf1 = fragWorld[1] / pos[1].w;
        vec3f pcf2 = fragWorld[2] / pos[2].w;

        float inverse = invW0 * barycentric.x + invW1 * barycentric.y + invW2 * barycentric.z;
        vec3f worldFragPos = (pcf0 * barycentric.x + pcf1 * barycentric.y + pcf2 * barycentric.z) / inverse;

        float shadow = shadowCalc(worldFragPos);

        fragColour = vec4(ambient + (1.f - shadow), 1.f) * texture(2, face, *diffuse, barycentric);
        return true;
    }

    float shadowCalc(const vec3f &worldFragPos)
    {
        vec4f lightPos = lightSpaceMatrix * vec4(worldFragPos, 1.f);
        vec3f proj = vec3(lightPos) / lightPos.w;
        lightDir = normalise(lightDir);

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