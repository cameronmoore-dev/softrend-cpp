#include "assimp_mesh.h"

AssimpMesh::AssimpMesh(std::vector<AssimpVertex> vertices, std::vector<uint32_t> indices) : 
    m_vertices(vertices),
    m_indices(indices)
{
}