#pragma once

#include <algorithm>
#include <cmath>

#include "../IShader.h"
#include "../Matrix.h"
#include "../Utils.h"
#include "../Image.h"

#include "../Renderer.h"

class Suzanne_Shader : public IShader
{
public:
    Uniform<Mat4> model;
    Uniform<Mat4> view;
    Uniform<Mat4> projection;

    Uniform<Image> diffuseMap;
    Uniform<Image> normalMap;
    Uniform<Image> specularMap;

    // Mat4 lightProj;
    // Mat4 lightView;

    Vec3f lightDir{ 0, 0, -1 };
    Vec3f colour = Utils::hexToRGB(0xffffff);
    Vec3f ambient = Utils::hexToRGB(0x111111);

    bool isShadowPass = false;

private:
    void vertex(const Vertex& v, glVertex& copy) override
    {
        // if (isShadowPass)
        // {
        //     copy.position = Matrix::multiply(lightProj, Matrix::multiply(lightView, Matrix::multiply(model, v.position)));
        //     return;
        // }

        copy.uv = v.uv;
        copy.colour = v.colour;
        copy.normal = Matrix::multiplyVec3(*model, v.normal);
        copy.tangent = Matrix::multiplyVec3(*model, v.tangent);
        copy.bitangent = Matrix::multiplyVec3(*model, v.bitangent);

        copy.fragPos = Matrix::multiply(*model, v.position);
        copy.position = Matrix::multiply(*projection, Matrix::multiply(*view, Matrix::multiply(*model, v.position)));
    }

    bool fragment(const Vec3f& barycentric, const glTriangle& polygon, Vec4f& finalColour) override
    {
        // Vec4f fragWorldPos = polygon.v0.fragPos * barycentric.x + polygon.v1.fragPos * barycentric.y + polygon.v2.fragPos * barycentric.z;
        // Vec4f fragPos = Matrix::multiply(lightProj, Matrix::multiply(lightView, fragWorldPos));

        // /* Perspective Division for use with perspective projection */
        // fragPos.x /= fragPos.w;
        // fragPos.y /= fragPos.w;
        // fragPos.z /= fragPos.w;

        // Vec2f shadowMapUV;
        // shadowMapUV.x = fragPos.x * 0.5f + 0.5f;
        // shadowMapUV.y = fragPos.y * 0.5f + 0.5f;
        
        // int sx = shadowMapUV.x * 1024;
        // int sy = shadowMapUV.y * 1024;

        // float shadowDepth = Renderer::depthMap[sx + sy * 1024];
        // float currentDepth = fragPos.z;

        // float bias = std::max(0.05f * (1.0f - Utils::dot(normal, lightDir)), 0.005f);
        // float shadow = currentDepth - bias > shadowDepth ? 1.f : 0.f;

        Vec3f normal = sampleNormalMap(polygon, *normalMap, barycentric);
        // Vec3f normal = polygon.v0.normal * barycentric.x + polygon.v1.normal * barycentric.y + polygon.v2.normal * barycentric.z;
        normal = Utils::normalise(normal);

        float diff = std::max(0.f, Utils::dot(normal, lightDir));
        Vec3f diffuse = colour * diff;

        Vec3f halfwayDir = Utils::normalise(lightDir + lightDir);
        float spec = std::pow(std::max(Utils::dot(normal, halfwayDir), 0.f), 128);
        Vec3f specular = sampleSpecularMap(polygon, *specularMap, barycentric) * spec;

        Vec3f final_light = (ambient + (diffuse + specular) /* * (1.0f - shadow) */) * sampleDiffuseMap(polygon, *diffuseMap, barycentric);

        finalColour = Utils::toVec4(final_light);
        // finalColour = Utils::hexToRGBA(0xFFFFFFFF);
        return true;
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