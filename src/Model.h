#pragma once

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "vertex.h"

class Model
{
public:
    Model(const std::string& dir);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

private:
    void loadModel(const std::string& dir);
    void computeTangents();
};