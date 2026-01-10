#pragma once

#include <vector>

#include "vertex.h"

class AssimpMesh
{
public:
    AssimpMesh(std::vector<AssimpVertex> vertices);

    std::vector<AssimpVertex> m_vertices;
private:
};