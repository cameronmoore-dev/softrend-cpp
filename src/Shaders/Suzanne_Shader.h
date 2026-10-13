#pragma once

#include <algorithm>
#include <cmath>

#include "Renderer/shader.h"
#include "Renderer/renderer.h"

class Suzanne_Shader : public IShader
{
public:
    Uniform<mat4> model;
    Uniform<mat4> view;
    Uniform<mat4> projection;

    Uniform<Texture> diffuseMap;
    Uniform<Texture> normalMap;
    Uniform<Texture> specularMap;

    vec3f lightDir{ 0, 0, -1 };
    vec3f colour = { 1, 1, 1 };
    vec3f ambient = { 0.17f, 0.17f, 0.17f };

private:
    void vertex(float *vertex) override
    {
        vec3f position = getVertexAttribData<vec3f>(vertex, 0);
        vec3f normals  = getVertexAttribData<vec3f>(vertex, 1);
        
        vec3f normal = normalise(*model * normals);
        vertex[4] = normal.x;
        vertex[5] = normal.y;
        vertex[6] = normal.z;

        glPosition(0, vertex, (*projection * (*view * (*model * vec4(position, 1.f)))));
    }

    bool fragment(const vec3f& barycentric, const Face &face, vec4f& fragColour) override
    {
        // std::array<vec3f, 3> normals = getFaceAttrib<vec3f>(face, 1);

        vec3f normal = getNormalMapTexel(face, barycentric);
        // vec3f normal = normals[0] * barycentric.x + normals[1] * barycentric.y + normals[2] * barycentric.z;
        normal = normalise(normal);

        float diff = std::max(0.f, dot(normal, lightDir));
        vec3f diffuse = colour * diff;

        vec3f halfwayDir = normalise(lightDir + lightDir);
        float spec = std::pow(std::max(dot(normal, halfwayDir), 0.f), 128);
        vec3f specular = vec3(texture(2, face, *specularMap, barycentric) * spec);

        fragColour = vec4((ambient + (diffuse + specular)), 1.f) * texture(2, face, *diffuseMap, barycentric);
        return true;
    }

    vec3f getNormalMapTexel(const Face &face, const vec3f &barycentric)
    {
        std::array<vec3f, 3> normals    = getFaceAttrib<vec3f>(face, 1);
        std::array<vec3f, 3> tangents   = getFaceAttrib<vec3f>(face, 3);
        std::array<vec3f, 3> bitangents = getFaceAttrib<vec3f>(face, 4);
    
        // Perspective-correct TBN
        vec3f T = tangents[0] * barycentric.x + tangents[1] * barycentric.y + tangents[2] * barycentric.z;
        vec3f B = bitangents[0] * barycentric.x + bitangents[1] * barycentric.y + bitangents[2] * barycentric.z;
        vec3f N = normals[0] * barycentric.x + normals[1] * barycentric.y + normals[2] * barycentric.z;

        vec3f texel = vec3(texture(2, face, *normalMap, barycentric));
        vec3f normal = normalise(texel * 2.f - 1.f);
        mat3 tbn = constructTBN(T, B, N);

        return normalise(tbn * normal);
    }
};

// float max = 1.f;
// float min = 0.1f;
// float multi = 1.1f;
// float d = ((2.f * min * max) / (max + min - depth * (max - min)));
// float r = ((d - min) / (max - min)) * multi;
// r = std::clamp(r, 0.f, 1.f);

/* Flat Shading */
// Vec3f normal = Utils::getFaceNormal(v0.worldPos, v1.worldPos, v2.worldPos);

/* Gouraud Shading */
// Vec3f normal = polygon.v0.normal * barycentric.x + polygon.v1.normal * barycentric.y + polygon.v2.normal * barycentric.z;
// normal = Utils::normalise(normal);

/* World space normals from normal map */
//Vec3f normal = sampleNormalMap(polygon, *normalMap, barycentric);

// float diff = std::max(0.f, Utils::dot(normal, lightDir));
// Vec3f diffuse = colour * diff;


/* Phong Lighting Model - the lightDir used in the dot product is a substitute for the view direction */
// Vec3f reflectDir = Utils::reflect(-lightDir, normal);
// float spec = std::pow(std::max(Utils::dot(lightDir, reflectDir), 0.f), 64);
// float specStrength = 0.5f;
// Vec3f specular = colour * specStrength * spec;

/* Blinn-Phong Lighting Model */
// Vec3f halfwayDir = Utils::normalise(lightDir + lightDir);
// float spec = std::pow(std::max(Utils::dot(normal, halfwayDir), 0.f), 256);
// Vec3f specular = colour * spec;

// Utils::hexToRGB(sampleDiffuseMap(polygon, *texture, barycentric))

// uint32_t final = Utils::combineHex(Utils::rgbToHex(final_light), fog(idx, 0x111111), r);