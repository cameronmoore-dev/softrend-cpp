#pragma once

#include <vector>

#include "vertex.h"

class AssimpMesh
{
public:
    AssimpMesh(std::vector<AssimpVertex> vertices, std::vector<uint32_t> indices);

    std::vector<AssimpVertex> m_vertices;
    std::vector<uint32_t> m_indices;
private:
};