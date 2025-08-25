#include "Model.h"

#include <filesystem>
#include <iostream>

Model::Model(const std::string& dir)
{
    loadModel(dir);
}

void Model::loadModel(const std::string& dir)
{
    std::filesystem::path path = std::filesystem::current_path().string() + dir;
    std::ifstream file(path, std::ifstream::in);

    if (!file)
    {
        std::cerr << "ERROR::MODEL::failed to open file!" << std::endl;
        return;
    }

    std::string line;
    while (!file.eof())
    {
        std::getline(file, line);
        std::istringstream s_stream(line);
        std::string prefix;

        s_stream >> prefix;

        /* Vertex */
        if (prefix == "v")
        {
            Vec3f v;
            s_stream >> v.x >> v.y >> v.z;
            m_positions.emplace_back(v);
            m_vtxPositions.emplace_back(v);
        }

        /* Texture Coord */
        else if (prefix == "vt")
        {
            Vec2f vt;
            s_stream >> vt.x >> vt.y;
            m_texCoords.emplace_back(vt);
        }

        /* Normals */
        else if (prefix == "vn")
        {
            Vec3f vn;
            s_stream >> vn.x >> vn.y >> vn.z;
            m_normals.emplace_back(vn);
        }

        /* Faces */
        else if (prefix == "f")
        {
            Face face;
            std::string vtxStr;
            while (s_stream >> vtxStr)
            {
                std::istringstream vs_stream(vtxStr);

                std::string v, t, n;
                FaceIndex fidx = { -1, -1, -1};

                size_t v_pos = vtxStr.find("/");
                size_t t_pos = vtxStr.find("/", v_pos + 1);

                if (v_pos != std::string::npos)
                {
                    v = vtxStr.substr(0, v_pos);
                    if (t_pos != std::string::npos)
                    {
                        t = vtxStr.substr(v_pos + 1, t_pos - v_pos - 1);
                        n = vtxStr.substr(t_pos + 1);
                    }
                    else 
                    {
                        t = vtxStr.substr(v_pos + 1);
                    }
                }
                else 
                {
                    v = vtxStr;
                }

                if (!v.empty()) fidx.positionIndex = std::stoi(v) - 1;
                if (!t.empty()) fidx.texCoordIndex = std::stoi(t) - 1;
                if (!n.empty()) fidx.normalIndex = std::stoi(n) - 1;

                face.indices.emplace_back(fidx);
            }

            m_faces.emplace_back(face);
        }
    }
}

void Model::draw(FrameBuffer& frame)
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

    for (UINT i = 0; i < m_positions.size(); i++)
    {
        m_positions[i] = m_vtxPositions[i];
        Vec4f proj = Matrix::multiply(mvp, m_positions[i]);
        if (proj.w != 0.f)
        {
            proj.x /= proj.w;
            proj.y /= proj.w;
            proj.z /= proj.w;
        }

        m_positions[i].x = (proj.x + 1) * 0.5f * frame.width;
        m_positions[i].y = (1 - proj.y) * 0.5f * frame.height;
    }

    for (UINT f = 0; f < m_faces.size(); f++)
    {
        std::vector<FaceIndex> fIndex = m_faces[f].indices;
        std::vector<Vec3f> vtxPositions = {};
        for (UINT p = 0; p < 3; p++)
        {
            vtxPositions.emplace_back(m_positions[fIndex[p].positionIndex]);
        }

        boundingBox(vtxPositions[0], vtxPositions[1], vtxPositions[2]);
        rasterise(frame, vtxPositions[0], vtxPositions[1], vtxPositions[2]);
    }
}

void Model::boundingBox(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    FLOAT leftMost = std::min(a.x, std::min(b.x, c.x));
    FLOAT bottomMost = std::min(a.y, std::min(b.y, c.y));
    FLOAT rightMost = std::max(a.x, std::max(b.x, c.x));
    FLOAT topMost = std::max(a.y, std::max(b.y, c.y));

    m_bbMin = { leftMost, bottomMost, 0 };
    m_bbMax = { rightMost, topMost, 0 };
}

Vec3f Model::barycentric(const Vec2i& p, const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    Vec3f _p;
    _p.x = p.x;
    _p.y = p.y;

    FLOAT denom = ((b.y-c.y)*(a.x-c.x)+(c.x-b.x)*(a.y-c.y));
    FLOAT w1 = ((b.y-c.y)*(p.x-c.x)+(c.x-b.x)*(_p.y-c.y)) / denom;
    FLOAT w2 = ((c.y-a.y)*(_p.x-c.x)+(a.x-c.x)*(p.y-c.y)) / denom;
    FLOAT w3 = 1.0f - w1 - w2;

    return { w1, w2, w3 };
}

void Model::rasterise(FrameBuffer& frame, const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    Vec2i p;
    for (p.y = m_bbMin.y; p.y <= m_bbMax.y; p.y++)
    {
        for (p.x = m_bbMin.x; p.x <= m_bbMax.x; p.x++)
        {
            if (p.x < 0 || p.y < 0 || p.x >= frame.width || p.y >= frame.height)
            {
                continue;
            }

            Vec3f point = barycentric(p, a, b, c);
            if (point.x >= 0 && point.y >= 0 && point.z >= 0)
            {
                frame.pixels[p.x + p.y * frame.width] = Utils::rasteriseColour(
                    0xff0000,
                    0x00ff00,
                    0x0000ff,
                    point.x, point.y, point.z);
            }
        }
    }
}