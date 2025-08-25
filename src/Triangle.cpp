#include "Triangle.h"

#include <cstdlib>
#include <minwindef.h>
#include <cmath>

#include <iostream>

Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3)
{
    m_vertices.emplace_back(v1);
    m_vertices.emplace_back(v2);
    m_vertices.emplace_back(v3);

    m_vtxPositions.emplace_back(v1.position);
    m_vtxPositions.emplace_back(v2.position);
    m_vtxPositions.emplace_back(v3.position);
}

void Triangle::draw(FrameBuffer& fb)
{
    m_angle+=0.1;
    Mat4 rotateZ = Matrix::rotate(Utils::degToRad(m_angle), 2);
    Mat4 rotateY = Matrix::rotate(Utils::degToRad(m_angle), 1);
    Mat4 rotateX = Matrix::rotate(Utils::degToRad(m_angle / 2), 0);

    m_rotate = Matrix::multiply(rotateZ, rotateY);
    m_rotate = Matrix::multiply(m_rotate, rotateX);

    Mat4 model = Matrix::multiply(m_translate, m_rotate);
    model = Matrix::multiply(model, m_scale);
    Mat4 mv = Matrix::multiply(m_view, model);

    Mat4 mvp = Matrix::multiply(m_persp, mv);

    for (UINT i = 0; i < m_vertices.size(); i++)
    {
        m_vertices[i].position = m_vtxPositions[i];

        Vec4f proj = Matrix::multiply(mvp, m_vertices[i].position);
        if (proj.w != 0.f)
        {
            proj.x /= proj.w;
            proj.y /= proj.w;
            proj.z /= proj.w;
        }

        m_vertices[i].position.x = (proj.x + 1) * 0.5f * fb.width;
        m_vertices[i].position.y = (1 - proj.y) * 0.5f * fb.height;
    }

    boundingBox();

    Vec2i p;
    for (p.y = m_bbMin.y; p.y <= m_bbMax.y; p.y++)
    {
        for (p.x = m_bbMin.x; p.x <= m_bbMax.x; p.x++)
        {
            if (p.x < 0 || p.y < 0 || p.x >= fb.width || p.y >= fb.height)
            {
                continue;
            }

            Vec3f point = barycentric(p);
            if (point.x >= 0 && point.y >= 0 && point.z >= 0) {
                fb.pixels[p.x + p.y * fb.width] = Utils::rasteriseColour(
                    m_vertices[0].colour,
                    m_vertices[1].colour,
                    m_vertices[2].colour,
                    point.x, point.y, point.z);
            }
        }
    }
}

void Triangle::boundingBox()
{
    FLOAT leftMost = std::min(m_vertices[0].position.x, std::min(m_vertices[1].position.x, m_vertices[2].position.x));
    FLOAT bottomMost = std::min(m_vertices[0].position.y, std::min(m_vertices[1].position.y, m_vertices[2].position.y));
    FLOAT rightMost = std::max(m_vertices[0].position.x, std::max(m_vertices[1].position.x, m_vertices[2].position.x));
    FLOAT topMost = std::max(m_vertices[0].position.y, std::max(m_vertices[1].position.y, m_vertices[2].position.y));

    m_bbMin = { leftMost, bottomMost, 0 };
    m_bbMax = { rightMost, topMost, 0 };
}

Vec3f Triangle::barycentric(const Vec2i& p)
{
    Vec3f a = m_vertices[0].position;
    Vec3f b = m_vertices[1].position;
    Vec3f c = m_vertices[2].position;
    Vec3f _p;
    _p.x = p.x;
    _p.y = p.y;

    FLOAT denom = ((b.y-c.y)*(a.x-c.x)+(c.x-b.x)*(a.y-c.y));
    FLOAT w1 = ((b.y-c.y)*(p.x-c.x)+(c.x-b.x)*(_p.y-c.y)) / denom;
    FLOAT w2 = ((c.y-a.y)*(_p.x-c.x)+(a.x-c.x)*(p.y-c.y)) / denom;
    FLOAT w3 = 1.0f - w1 - w2;

    return { w1, w2, w3 };
}