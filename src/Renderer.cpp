#include "Renderer.h"

#include <algorithm>
#include <cmath>

Renderer::Renderer(Window& window) : 
    m_context(window)
{
}

void Renderer::draw(const std::vector<Vertex>& vertices, Image* texture, Camera& camera, const Mat4& model)
{
    Vec3f centre{0, 0, 0};
    float radius = 1.f;
    if (!camera.isSphereInsideFrustum(model, centre, radius))
    {
        return;
    }

    std::vector<Renderer::RasterVertex> rVertices;
    for (uint32_t i = 0; i < vertices.size(); i+=3)
    {
        Renderer::Triangle triangle = toModelSpace(vertices[i], vertices[i+1], vertices[i+2], model);
        std::vector<Vertex> clipped = toViewSpace(triangle, camera);
        for (Vertex& v : clipped)
        {
            rVertices.emplace_back(toScreenSpace(v, camera.projection()));
        }
    }

    for (uint32_t i = 0; i < rVertices.size(); i+=3)
    {
        rasterise(rVertices[i], rVertices[i+1], rVertices[i+2], texture);
    }
}

Renderer::Triangle Renderer::toModelSpace(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Mat4& model)
{
    Renderer::Triangle modelTri{ v0, v1, v2 };

    modelTri.v0.worldPos = Matrix::multiplyVec3(model, v0.position);
    modelTri.v1.worldPos = Matrix::multiplyVec3(model, v1.position);
    modelTri.v2.worldPos = Matrix::multiplyVec3(model, v2.position);

    modelTri.v0.normal = Utils::normalise(Matrix::multiplyVec3AsNormal(model, v0.normal));
    modelTri.v1.normal = Utils::normalise(Matrix::multiplyVec3AsNormal(model, v1.normal));
    modelTri.v2.normal = Utils::normalise(Matrix::multiplyVec3AsNormal(model, v2.normal));

    return modelTri;
}

std::vector<Vertex> Renderer::toViewSpace(const Renderer::Triangle& tri, const Camera& camera)
{
    Renderer::Triangle viewTri = tri;
    
    viewTri.v0.position = Matrix::multiplyVec3(camera.view(), tri.v0.worldPos);
    viewTri.v1.position = Matrix::multiplyVec3(camera.view(), tri.v1.worldPos);
    viewTri.v2.position = Matrix::multiplyVec3(camera.view(), tri.v2.worldPos);

    // viewTri.v0.normal = Utils::normalise(Matrix::multiplyVec3AsNormal(camera.view(), tri.v0.normal));
    // viewTri.v1.normal = Utils::normalise(Matrix::multiplyVec3AsNormal(camera.view(), tri.v1.normal));
    // viewTri.v2.normal = Utils::normalise(Matrix::multiplyVec3AsNormal(camera.view(), tri.v2.normal));

    return nearPlaneClip(viewTri.v0, viewTri.v1, viewTri.v2, camera.nearPlane(), camera.farPlane());
}

Renderer::RasterVertex Renderer::toScreenSpace(const Vertex& vertex, const Mat4& projection)
{
    Vec4f clipSpace = Matrix::multiply(projection, vertex.position);

    /* Perspective Division */
    clipSpace.x /= clipSpace.w;
    clipSpace.y /= clipSpace.w;
    clipSpace.z /= clipSpace.w;

    /* Screen Space */
    Vec3f screenPos;
    screenPos.x = (clipSpace.x + 1) * 0.5f * m_context.m_width;
    screenPos.y = (clipSpace.y + 1) * 0.5f * m_context.m_height;
    screenPos.z = clipSpace.z;

    /* Perspective Correction for Textures */
    Renderer::RasterVertex rv;
    rv.worldPos = vertex.worldPos;
    rv.viewPos = vertex.position;
    rv.normal = vertex.normal;
    rv.screenPos = screenPos;
    rv.inverseZ = 1.f / clipSpace.w;
    rv.uvOverZ = vertex.texCoords * rv.inverseZ;
    rv.colour = vertex.colour;
    return rv;
}

void Renderer::rasterise(const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2, Image* image)
{
    /* Back-face Culling */
    float winding = edgeFunction(v0.screenPos, v1.screenPos, v2.screenPos);
    if (winding <= 0)
    {
        return;
    }

    boundingBox(v0.screenPos, v1.screenPos, v2.screenPos);

    Vec2i p;
    for (p.y = m_bbMin.y; p.y <= m_bbMax.y; p.y++)
    {
        for (p.x = m_bbMin.x; p.x <= m_bbMax.x; p.x++)
        {
            if (p.x < 0 || p.y < 0 || p.x >= m_context.m_width || p.y >= m_context.m_height)
            {
                continue;
            }

            Vec3f point = barycentric(p, v0.screenPos, v1.screenPos, v2.screenPos);
            if (point.x >= 0 && point.y >= 0 && point.z >= 0)
            {
                float z = v0.screenPos.z * point.x + v1.screenPos.z * point.y + v2.screenPos.z * point.z;
                uint32_t idx = p.x + p.y * m_context.m_width;
                if (z < m_context.m_depthBuffer[idx])
                {
                    m_context.m_depthBuffer[idx] = z;

                    if (image)
                    {
                        /* Move all this into a fragment shader */
                        float max = 1.f;
                        float min = 0.1f;
                        float multi = 1.1f;
                        float d = ((2.f * min * max) / (max + min - z * (max - min)));
                        float r = ((d - min) / (max - min)) * multi;
                        r = std::clamp(r, 0.f, 1.f);

                        Vec3f lightDir{ 0.f, 0.f, -1.f };
                        Vec3f lightColour = Utils::hexToRGB(0xffffff);

                        /* Flat Shading */
                        // Vec3f normal = Utils::getFaceNormal(v0.worldPos, v1.worldPos, v2.worldPos);

                        /* Gouraud Shading */
                        Vec3f normal = v0.normal * point.x + v1.normal * point.y + v2.normal * point.z;
                        normal = Utils::normalise(normal);

                        float diff = std::max(0.f, Utils::dot(normal, lightDir));
                        Vec3f diffuse = lightColour * diff;

                        /* Phong Lighting Model - the lightDir used in the dot product is a substitute for the view direction */
                        // Vec3f reflectDir = Utils::reflect(-lightDir, normal);
                        // float spec = std::pow(std::max(Utils::dot(lightDir, reflectDir), 0.f), 64);
                        // float specStrength = 0.5f;
                        // Vec3f specular = lightColour * specStrength * spec;

                        /* Blinn-Phong Lighting Model */
                        Vec3f halfwayDir = Utils::normalise(lightDir + lightDir);
                        float spec = std::pow(std::max(Utils::dot(normal, halfwayDir), 0.f), 256);
                        Vec3f specular = lightColour * spec;

                        Vec3f final_light = (Utils::hexToRGB(0x111111) + diffuse + specular) * Utils::hexToRGB(texel(v0, v1, v2, *image, point));

                        //uint32_t final = Utils::combineHex(Utils::rgbToHex(final_light), fog(idx, 0x111111), r);

                        m_context.m_backBuffer[idx] = Utils::rgbToHex(final_light);
                    }
                    else
                    {
                        m_context.m_backBuffer[idx] = Utils::rasteriseColour(v0.colour, v1.colour, v2.colour, point.x, point.y, point.z);
                    }
                }
            }
        }
    }
}

void Renderer::boundingBox(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float leftMost   = std::min(a.x, std::min(b.x, c.x));
    float bottomMost = std::min(a.y, std::min(b.y, c.y));
    float rightMost  = std::max(a.x, std::max(b.x, c.x));
    float topMost    = std::max(a.y, std::max(b.y, c.y));

    m_bbMin = { leftMost, bottomMost };
    m_bbMax = { rightMost, topMost };
}

Vec3f Renderer::barycentric(const Vec2i& p, const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float denom = ((b.y-c.y)*(a.x-c.x)+(c.x-b.x)*(a.y-c.y));
    float w1 = ((b.y-c.y)*(p.x-c.x)+(c.x-b.x)*(p.y-c.y)) / denom;
    float w2 = ((c.y-a.y)*(p.x-c.x)+(a.x-c.x)*(p.y-c.y)) / denom;
    float w3 = 1.0f - w1 - w2;

    return { w1, w2, w3 };
}

uint32_t Renderer::texel(const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2, const Image& image, const Vec3f& weights)
{
    /* Perspective-correct interpolation */

    float invInterp = v0.inverseZ * weights.x +
                      v1.inverseZ * weights.y +
                      v2.inverseZ * weights.z;

    Vec2f uvOverZInterp = v0.uvOverZ * weights.x +
                          v1.uvOverZ * weights.y +
                          v2.uvOverZ * weights.z;

    Vec2f finalUV = uvOverZInterp / invInterp;

    int32_t texX = finalUV.x * (image.size().x - 1);
    int32_t texY = finalUV.y * (image.size().y - 1);

    texX = std::clamp(texX, 0, image.size().x - 1);
    texY = std::clamp(texY, 0, image.size().y - 1);

    return image.pixel(texX + texY * image.size().x);
}

float Renderer::edgeFunction(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

Vertex Renderer::interpolate(const Vertex& a, const Vertex& b, float t)
{
    Vertex out;
    out.position = a.position + (b.position - a.position) * t;
    out.texCoords = a.texCoords + (b.texCoords - a.texCoords) * t;

    return out;
}

std::vector<Vertex> Renderer::nearPlaneClip(const Vertex& a, const Vertex& b, const Vertex& c, float zNear, float zFar)
{
    Vertex triangle[3] = { a, b, c };

    bool inside[3] = { 
        a.position.z <= -zNear, 
        b.position.z <= -zNear, 
        c.position.z <= -zNear };
        
    int inCount = inside[0] + inside[1] + inside[2];

    /* Triangle is fully outside of view */
    if (inCount == 0)
    {
        return {};
    }

    /* Triangle is fully inside of view */
    if (inCount == 3)
    {
        return { a, b, c };
    }

    if (inCount == 1)
    {
        /* Found out which vertex is inside the view */
        int inVert = inside[0] ? 0 : inside[1] ? 1 : 2;

        /* A is inside, the others are not */
        Vertex& A = triangle[inVert];
        Vertex& B = triangle[(inVert + 1) % 3];
        Vertex& C = triangle[(inVert + 2) % 3];

        /* Scalar to use for finding the intersection points */
        float tAB = (-zNear - A.position.z) / (B.position.z - A.position.z);
        float tAC = (-zNear - A.position.z) / (C.position.z - A.position.z);

        /* Intersection points for each line between the inside vertex and both outside */
        B = interpolate(A, B, tAB);
        C = interpolate(A, C, tAC);

        return { A, B, C };
    }

    if (inCount == 2)
    {
        /* Find which vertex is outside the view */
        int outVert = !inside[0] ? 0 : (!inside[1] ? 1 : 2);
        Vertex& A = triangle[(outVert + 1) % 3];
        Vertex& B = triangle[(outVert + 2) % 3];
        Vertex& C = triangle[outVert];

        /* Scalar used for the line intersection between the vertex that's outside and the two that are in */
        float tAC = (-zNear - A.position.z) / (C.position.z - A.position.z);
        float tBC = (-zNear - B.position.z) / (C.position.z - B.position.z);

        /* Intersection points for each line between the outside vertex and both inside */
        Vertex clippedA = interpolate(A, C, tAC);
        Vertex clippedB = interpolate(B, C, tBC);

        /* Two new triangles made from the four vertices left after clipping C */
        return { A, B, clippedA,
                 B, clippedB, clippedA };
    }

    /* Return empty */
    return {};
}

uint32_t Renderer::fog(uint32_t idx, uint32_t fogColour)
{
    float depth = m_context.m_depthBuffer[idx];
    if (depth <= 0.f || depth >= 1.f)
    {
        return fogColour;
    }

    return Utils::combineHex(m_context.m_backBuffer[idx], fogColour, 1);
}

// void Renderer::linearFog(float zNear, float zFar, float fogStart, float fogEnd)
// {
//     _Fog(zNear, zFar, fogStart, fogEnd, 0);
// }

// void Renderer::exponentialFog(float zNear, float zFar, float density)
// {
//     _Fog(zNear, zFar, 0, 0, density);
// }

// void Renderer::_Fog(float zNear, float zFar, float fogStart, float fogEnd, float density)
// {
//     for (uint32_t i = 0; i < m_context.m_width*m_context.m_height; i++)
//     {
//         uint32_t& currentPixel = m_context.m_backBuffer[i];
//         float depth = m_context.m_depthBuffer[i];

//         /* 1.1f to give some leeway for objects clipping the far plane */
//         if (depth <= 0 || depth >= 1.1f)
//         {
//             continue;
//         }
        
//         float distance = (2.f * zNear * zFar) / (zFar + zNear - depth * (zFar - zNear));
//         float factor = 0.f;

//         if (density == 0)
//         {
//             /* Linear */
//             factor = (fogEnd - distance) / (fogEnd - fogStart);
//         }
//         else
//         {
//             /* Exponential */
//             factor = 1.f / std::pow(math::e, distance * density);
//         }

//         Vec3f rgb = Utils::hexToRGB(currentPixel);

//         /* Window clear colour */
//         Vec3f fogColour = Utils::hexToRGB(0x111111);

//         rgb.x = rgb.x * factor + fogColour.x * (1.f - factor);
//         rgb.y = rgb.y * factor + fogColour.y * (1.f - factor);
//         rgb.z = rgb.z * factor + fogColour.z * (1.f - factor);

//         currentPixel = Utils::rgbToHex(rgb);
//     }
// }