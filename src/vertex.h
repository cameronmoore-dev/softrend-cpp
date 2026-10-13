#pragma once

#include "Renderer/math.hpp"

struct Vertex
{
    vec4f position;
    vec3f normal;
    vec2f uv;
    vec3f tangent;
    vec3f bitangent;
    vec3f colour;
};

struct AssimpVertex
{
    vec4f position;
    vec3f normal;
    vec2f uv;
    vec3f fragPos;
};