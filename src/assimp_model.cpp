#include "assimp_model.h"

void AssimpModel::loadModel(const std::string &path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        printf("ERROR::ASSIMP::%s", importer.GetErrorString());
        return;
    }

    processNode(scene->mRootNode, scene);
}

void AssimpModel::processNode(aiNode *node, const aiScene *scene)
{
    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.emplace_back(processMesh(mesh));
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

AssimpMesh AssimpModel::processMesh(aiMesh *mesh)
{
    std::vector<AssimpVertex> vertices;
    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        AssimpVertex vertex{};
        vertex.position = vec4f{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.normal   = vec3f{ mesh->mNormals[i].x,  mesh->mNormals[i].y,  mesh->mNormals[i].z};
        vertex.uv       = vec2f{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

        vertices.emplace_back(vertex);
    }

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++)
        {
            indices.emplace_back(face.mIndices[j]);
        }
    }

    return AssimpMesh(vertices, indices);
}