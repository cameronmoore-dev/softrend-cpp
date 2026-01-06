#pragma once

#include <vector>
#include <string>

#include "vertex.h"

class Model
{
public:
    Model(const std::string& dir);

    std::vector<Vertex> vertices;

private:
    void loadModel(const std::string& dir);
    void computeTangents();
};