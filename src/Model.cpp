#include "Model.h"

#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Utils.h"

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
                std::string v, t, n;

                size_t v_pos = vtxStr.find("/");
                size_t t_pos = vtxStr.find("/", v_pos + 1);
                size_t n_pos = vtxStr.find("/", v_pos + 3);

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
                if (t.size() > 0) vtx.uv = texCoords[std::stoi(t) - 1];
                if (n.size() > 0) vtx.normal = normals[std::stoi(n) - 1];

                vertices.emplace_back(vtx);
            }
        }
    }

    computeTangents();
}

void Model::computeTangents()
{
    for (uint32_t i = 0; i < vertices.size(); i+=3)
    {
        Vec3f pos0 = vertices[i].position;
        Vec3f pos1 = vertices[i+1].position;
        Vec3f pos2 = vertices[i+2].position;

        Vec2f uv0 = vertices[i].uv;
        Vec2f uv1 = vertices[i+1].uv;
        Vec2f uv2 = vertices[i+2].uv;

        Vec3f edge1 = pos1 - pos0;
        Vec3f edge2 = pos2 - pos0;

        Vec2f delta1 = uv1 - uv0;
        Vec2f delta2 = uv2 - uv0;

        float f = 1.f / (delta1.x * delta2.y - delta2.x * delta1.y);

        Vec3f t, b;

        t.x = (delta2.y * edge1.x - delta1.y * edge2.x) * f;
        t.y = (delta2.y * edge1.y - delta1.y * edge2.y) * f;
        t.z = (delta2.y * edge1.z - delta1.y * edge2.z) * f;

        b.x = (-delta2.x * edge1.x + delta1.x * edge2.x) * f;
        b.y = (-delta2.x * edge1.y + delta1.x * edge2.y) * f;
        b.z = (-delta2.x * edge1.z + delta1.x * edge2.z) * f;

        vertices[i].tangent += t;
        vertices[i+1].tangent += t;
        vertices[i+2].tangent += t;

        vertices[i].bitangent += b;
        vertices[i+1].bitangent += b;
        vertices[i+2].bitangent += b;
    }

    for (Vertex& vertex : vertices)
    {
        vertex.tangent = Utils::normalise(vertex.tangent);
        vertex.bitangent = Utils::normalise(vertex.bitangent);
    }
}