#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "Structs.h"
#include "Matrix.h"
#include "Utils.h"

class Model
{
public:
    Model(const std::string& dir);

    void draw(FrameBuffer& frame, FLOAT* depth);

    const size_t numVertices() const    { return m_positions.size(); }
    const size_t numTexCoords() const   { return m_texCoords.size(); }
    const size_t numNormals() const     { return m_normals.size(); }
    const size_t numFaces() const       { return m_faces.size(); }

private:
    std::vector<Vec3f> m_vtxPositions = {};

    std::vector<Vec3f> m_positions;
    std::vector<Vec2f> m_texCoords;
    std::vector<Vec3f> m_normals;
    std::vector<Face>  m_faces;

    Vec3f m_bbMin;
    Vec3f m_bbMax;

    FLOAT m_angle = 0.f;

    Mat4 m_persp = Matrix::perspective(Utils::degToRad(45.f), 640.f/480.f, 0.1f, 100.f);
    Mat4 m_ortho = Matrix::orthographic(-3.2, 3.2, -2.4, 2.4, 0.1f, 100.f);

    Mat4 m_view = Matrix::translate({ 0.f, 0.f, -7.f });

    Mat4 m_translate = Matrix::translate({0, 0, 1});
    Mat4 m_rotate = Matrix::rotate(Utils::degToRad(m_angle), 2);
    Mat4 m_scale = Matrix::scale({1, 1, 1});

private:
    void loadModel(const std::string& dir);

    void boundingBox(const Vec3f& a, const Vec3f& b, const Vec3f& c);
    Vec3f barycentric(const Vec2i& p, const Vec3f& a, const Vec3f& b, const Vec3f& c);
    void rasterise(FrameBuffer& frame, FLOAT* depth, const Vec3f& a, const Vec3f& b, const Vec3f& c);
};