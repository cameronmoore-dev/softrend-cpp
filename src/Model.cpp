#include "Model.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

Model::Model(const std::string& dir)
{
    loadModel(dir);
    //image.write("output.tga");
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
    
    std::vector<Vec3f> positions;
    std::vector<Vec2f> texCoords;
    std::vector<Vec3f> normals;

    std::string line;
    while (!file.eof())
    {
        std::getline(file, line);
        std::istringstream s_stream(line);
        std::string prefix;

        s_stream >> prefix;

        Vertex vtx;

        /* Vertex */
        if (prefix == "v")
        {
            Vec3f v;
            s_stream >> v.x >> v.y >> v.z;
            positions.emplace_back(v);
        }

        /* Texture Coord */
        else if (prefix == "vt")
        {
            Vec2f vt;
            s_stream >> vt.x >> vt.y;
            texCoords.emplace_back(vt);
        }

        /* Normals */
        else if (prefix == "vn")
        {
            Vec3f vn;
            s_stream >> vn.x >> vn.y >> vn.z;
            normals.emplace_back(vn);
        }

        /* Faces */
        else if (prefix == "f")
        {
            std::string vtxStr;
            while (s_stream >> vtxStr)
            {
                std::istringstream vs_stream(vtxStr);

                std::string v, t, n;

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

                vtx.position = positions[std::stoi(v) - 1];
                vtx.texCoords = texCoords[std::stoi(t) - 1];

                vertices.emplace_back(vtx);
            }
        }
    }
}

// void Model::draw(FrameBuffer& frame, FLOAT* depth)
// {
//     for (const Face& face : m_faces)
//     {
//         std::vector<FaceIndex> fIdx = face.indices;
//         Tri triangle;

//         Vec3f normals = Utils::getViewSpaceNormals(mv, m_vtxPositions[fIdx[0].positionIndex], m_vtxPositions[fIdx[1].positionIndex], m_vtxPositions[fIdx[2].positionIndex]);
//         FLOAT intensity = std::max(0.f, Utils::dot(Utils::normalise(normals), Utils::normalise(-m_lightDir)));

//         FLOAT invWs[3] = {};
//         Vec2f uvOverWs[3] = {};

//         for (UINT i = 0; i < fIdx.size(); i++)
//         {
//             /* m_positions[] (pos) are screen-space positions after the projection multiplication */
//             /* m_wldPosition[] are always world-space positions as they set when the model loads and get transformed */
//             Vec3f& pos = m_positions[fIdx[i].positionIndex] = m_vtxPositions[fIdx[i].positionIndex];

//             Vec4f proj = Matrix::multiply(mvp, pos);

//             invWs[i] = 1.0f/proj.w;
//             Vec2f uv = m_texCoords[fIdx[i].texCoordIndex];

//             uvOverWs[i] = uv * invWs[i];

//             if (proj.w != 0.f)
//             {
//                 proj.x /= proj.w;
//                 proj.y /= proj.w;
//                 proj.z /= proj.w;
//             }
    
//             pos.x = (proj.x + 1) * 0.5f * frame.width;
//             pos.y = (1 - proj.y) * 0.5f * frame.height;
//         }

//         triangle.v0.position = m_positions[fIdx[0].positionIndex];
//         triangle.v1.position = m_positions[fIdx[1].positionIndex];
//         triangle.v2.position = m_positions[fIdx[2].positionIndex];

//         triangle.v0.texCoords = m_texCoords[fIdx[0].texCoordIndex];
//         triangle.v1.texCoords = m_texCoords[fIdx[1].texCoordIndex];
//         triangle.v2.texCoords = m_texCoords[fIdx[2].texCoordIndex];

//         triangle.v0.invW = invWs[0];
//         triangle.v1.invW = invWs[1];
//         triangle.v2.invW = invWs[2];

//         triangle.v0.uvOverW = uvOverWs[0];
//         triangle.v1.uvOverW = uvOverWs[1];
//         triangle.v2.uvOverW = uvOverWs[2];
//     }
// }