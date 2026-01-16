#pragma once

#include <algorithm>
#include <cmath>

#include "echo-renderer/shader.h"
#include "echo-renderer/renderer.h"

class Suzanne_Shader : public echo::IShader
{
public:
    Uniform<echo::mat4> model;
    Uniform<echo::mat4> view;
    Uniform<echo::mat4> projection;

    Uniform<echo::Texture> diffuseMap;
    Uniform<echo::Texture> normalMap;
    Uniform<echo::Texture> specularMap;

    echo::vec3f lightDir{ 0, 0, -1 };
    echo::vec3f colour = { 1, 1, 1 };
    echo::vec3f ambient = { 0.17f, 0.17f, 0.17f };

private:
    void vertex(float *vertex) override
    {
        echo::vec3f position = getVertexAttribData<echo::vec3f>(vertex, 0);
        echo::vec3f normals  = getVertexAttribData<echo::vec3f>(vertex, 1);
        
        echo::vec3f normal = echo::normalise(*model * normals);
        vertex[4] = normal.x;
        vertex[5] = normal.y;
        vertex[6] = normal.z;

        glPosition(0, vertex, (*projection * (*view * (*model * vec4(position, 1.f)))));
    }

    bool fragment(const echo::vec3f& barycentric, const echo::Face &face, echo::vec4f& fragColour) override
    {
        std::array<echo::vec3f, 3> normals = getFaceAttrib<echo::vec3f>(face, 1);

        echo::vec3f normal = getNormalMapTexel(face, barycentric);
        // echo::vec3f normal = normals[0] * barycentric.x + normals[1] * barycentric.y + normals[2] * barycentric.z;
        normal = normalise(normal);

        float diff = std::max(0.f, echo::dot(normal, lightDir));
        echo::vec3f diffuse = colour * diff;

        echo::vec3f halfwayDir = normalise(lightDir + lightDir);
        float spec = std::pow(std::max(dot(normal, halfwayDir), 0.f), 128);
        echo::vec3f specular = vec3(texture(2, face, *specularMap, barycentric) * spec);

        fragColour = vec4((ambient + (diffuse )), 1.f) * texture(2, face, *diffuseMap, barycentric);
        return true;
    }

    echo::vec3f getNormalMapTexel(const echo::Face &face, const echo::vec3f &barycentric)
    {
        std::array<echo::vec3f, 3> normals    = getFaceAttrib<echo::vec3f>(face, 1);
        std::array<echo::vec3f, 3> tangents   = getFaceAttrib<echo::vec3f>(face, 3);
        std::array<echo::vec3f, 3> bitangents = getFaceAttrib<echo::vec3f>(face, 4);
    
        // Perspective-correct TBN
        echo::vec3f T = tangents[0] * barycentric.x + tangents[1] * barycentric.y + tangents[2] * barycentric.z;
        echo::vec3f B = bitangents[0] * barycentric.x + bitangents[1] * barycentric.y + bitangents[2] * barycentric.z;
        echo::vec3f N = normals[0] * barycentric.x + normals[1] * barycentric.y + normals[2] * barycentric.z;

        echo::vec3f texel = vec3(texture(2, face, *normalMap, barycentric));
        echo::vec3f normal = echo::normalise(texel * 2.f - 1.f);
        echo::mat3 tbn = constructTBN(T, B, N);

        return echo::normalise(tbn * normal);
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