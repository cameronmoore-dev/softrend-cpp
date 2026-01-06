#pragma once

#include <algorithm>
#include <cmath>

#include "../IShader.h"
#include "../Matrix.h"
#include "../Renderer.h"

class Suzanne_Shader : public IShader
{
public:
    Uniform<Mat4> model;
    Uniform<Mat4> view;
    Uniform<Mat4> projection;

    Uniform<Texture> diffuseMap;
    Uniform<Texture> normalMap;
    Uniform<Texture> specularMap;

    // Mat4 lightProj;
    // Mat4 lightView;

    Vec3f lightDir{ 0, 0, -1 };
    Vec3f colour = { 1, 1, 1 };
    Vec3f ambient = { 0.17f, 0.17f, 0.17f };

    bool isShadowPass = false;

    Vec3f vertColour = {};

private:
    void vertex(float *vertex) override
    {
        Vec3f position = getVertexAttribData3(vertex, 0);
        Vec3f normals = getVertexAttribData3(vertex, 1);

        // if (isShadowPass)
        // {
        //     copy.position = Matrix::multiply(lightProj, Matrix::multiply(lightView, Matrix::multiply(model, v.position)));
        //     return;
        // }
        
        Vec3f normal = normalise(Matrix::multiplyVec3(*model, normals));
        vertex[4] = normal.x;
        vertex[5] = normal.y;
        vertex[6] = normal.z;
        glPosition(0, vertex, Matrix::multiply(*projection, Matrix::multiply(*view, Matrix::multiply(*model, position))));
    }

    bool fragment(const Vec3f& barycentric, const face_t &face, Vec4f& fragColour) override
    {
        std::array<Vec3f, 3> normals = getFaceAttrib(face, 1);

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

        // float bias = std::max(0.05f * (1.0f - dot(normal, lightDir)), 0.005f);
        // float shadow = currentDepth - bias > shadowDepth ? 1.f : 0.f;

        Vec3f normal = sampleNormalMap(face, *normalMap, barycentric);
        // Vec3f normal = normals[0] * barycentric.x + normals[1] * barycentric.y + normals[2] * barycentric.z;
        normal = normalise(normal);

        float diff = std::max(0.f, math::dot(normal, lightDir));
        Vec3f diffuse = colour * diff;

        Vec3f halfwayDir = normalise(lightDir + lightDir);
        float spec = std::pow(std::max(dot(normal, halfwayDir), 0.f), 64);
        Vec3f specular = sampleSpecularMap(face, *specularMap, barycentric) * spec;

        fragColour = vec4((ambient + (diffuse + specular) /* * (1.0f - shadow) */), 1.f) * sampleDiffuseMap(2, face, *diffuseMap, barycentric);
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