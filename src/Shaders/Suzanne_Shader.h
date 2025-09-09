#pragma once

#include <algorithm>
#include <cmath>

#include "../IShader.h"
#include "../Matrix.h"
#include "../Utils.h"
#include "../Image.h"

class Suzanne_Shader : public IShader
{
public:
    Mat4 projection;
    Mat4 view;
    Mat4 model;

    Vec3f lightDir { 0.f, 0.f, -1.f };
    Vec3f colour = Utils::hexToRGB(0xffffff);
    Vec3f ambient = Utils::hexToRGB(0x111111);

    Image* texture = nullptr;
    Image* normalMap = nullptr;

private:
    void vertex(const Vertex& v, glVertex& copy) override
    {
        copy.uv = v.uv;
        copy.colour = v.colour;
        copy.normal = Matrix::multiplyVec3(model, v.normal);
        copy.tangent = Matrix::multiplyVec3(model, v.tangent);
        copy.bitangent = Matrix::multiplyVec3(model, v.bitangent);

        copy.position = Matrix::multiply(projection, Matrix::multiply(view, Matrix::multiply(model, v.position)));
    }

    uint32_t fragment(const Vec3f& barycentric, const glTriangle& polygon, float depth) override
    {
        /* Gouraud Shading */
        // Vec3f normal = polygon.v0.normal * barycentric.x + polygon.v1.normal * barycentric.y + polygon.v2.normal * barycentric.z;
        // normal = Utils::normalise(normal);

        /* World space normals from normal map */
        Vec3f normal = sampleNormalMap(polygon, *normalMap, barycentric);

        float diff = std::max(0.f, Utils::dot(normal, lightDir));
        Vec3f diffuse = colour * diff;

        /* Blinn-Phong Lighting Model */
        Vec3f halfwayDir = Utils::normalise(lightDir + lightDir);
        float spec = std::pow(std::max(Utils::dot(normal, halfwayDir), 0.f), 256);
        Vec3f specular = colour * spec;
        
        Vec3f final_light = (ambient + diffuse + specular) * Utils::hexToRGB(sampleDiffuseMap(polygon, *texture, barycentric));

        return Utils::rgbToHex(final_light);
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


/* Phong Lighting Model - the lightDir used in the dot product is a substitute for the view direction */
// Vec3f reflectDir = Utils::reflect(-lightDir, normal);
// float spec = std::pow(std::max(Utils::dot(lightDir, reflectDir), 0.f), 64);
// float specStrength = 0.5f;
// Vec3f specular = colour * specStrength * spec;

// uint32_t final = Utils::combineHex(Utils::rgbToHex(final_light), fog(idx, 0x111111), r);