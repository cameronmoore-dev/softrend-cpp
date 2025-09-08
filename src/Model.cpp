#include "Model.h"

#include <filesystem>
#include <algorithm>
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
                if (t.size() > 0) vtx.texCoords = texCoords[std::stoi(t) - 1];
                if (n.size() > 0) vtx.normal = normals[std::stoi(n) - 1];

                vertices.emplace_back(vtx);
            }
        }
    }
}