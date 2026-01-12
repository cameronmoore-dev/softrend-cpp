#pragma once

#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "assimp_mesh.h"

class AssimpModel
{
public:
    AssimpModel() = default;

    void loadModel(const std::string &path);
    void processNode(aiNode *node, const aiScene *scene);
    AssimpMesh processMesh(aiMesh *mesh);

    std::vector<AssimpMesh> meshes;
private:
};