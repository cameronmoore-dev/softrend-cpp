#include "assimp_model.h"

void AssimpModel::loadModel(const std::string &path)
{
    Assimp::Importer imp;
    const aiScene *scene = imp.ReadFile(path, aiProcess_Triangulate);

    if (!scene)
    {
        printf("error loading model!\n");
        return;
    }

    processNode(scene->mRootNode, scene);
}

void AssimpModel::processNode(aiNode *node, const aiScene *scene)
{
    for (int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.emplace_back(processMesh(mesh, scene));
    }

    for (int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

AssimpMesh AssimpModel::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<AssimpVertex> vertices;

    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        AssimpVertex vertex;
        vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.f };
        vertex.normal   = { mesh->mNormals[i].x, mesh->mNormals[i].y,  mesh->mNormals[i].z};

        if (mesh->mTextureCoords[0])
        {
            vertex.uv   = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }
        else 
        {
            vertex.uv = { 0, 0 };
        }

        vertices.emplace_back(vertex);
    }

    return AssimpMesh(vertices);
}