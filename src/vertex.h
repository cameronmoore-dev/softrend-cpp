#pragma once

#include "echo-renderer/echo_math.h"

struct Vertex
{
    echo::vec4f position;
    echo::vec3f normal;
    echo::vec2f uv;
    echo::vec3f tangent;
    echo::vec3f bitangent;
    // uint32_t colour;
};

struct AssimpVertex
{
    echo::vec4f position;
    echo::vec3f normal;
    echo::vec2f uv;
    echo::vec3f fragPos;
};