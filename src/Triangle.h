// #pragma once

// #include <vector>
// #include <winerror.h>

// #include "Structs.h"
// #include "Utils.h"
// #include "Matrix.h"

// class Triangle
// {
// public:
//     Triangle(Vertex v1, Vertex v2, Vertex v3);

//     void draw(FrameBuffer& fb);

// private:
//     std::vector<Vertex> m_vertices;
//     std::vector<Vec3f> m_vtxPositions;

//     Vec3f m_bbMin;
//     Vec3f m_bbMax;

//     FLOAT m_angle = 0.0f;

//     Mat4 m_persp = Matrix::perspective(Utils::degToRad(45.f), 640.f/480.f, 0.1f, 100.f);
//     Mat4 m_ortho = Matrix::orthographic(-320, 320, -240, 240, 0.1f, 100.f);

//     Mat4 m_view = Matrix::translate({ 0.f, 0.f, -800.f });

//     Mat4 m_translate = Matrix::translate({0, 0, 0});
//     Mat4 m_rotate = Matrix::rotate(Utils::degToRad(m_angle), 2);
//     Mat4 m_scale = Matrix::scale({1, 1, 0});

// private:
//     void boundingBox();
//     Vec3f barycentric(const Vec2i& p);
// };