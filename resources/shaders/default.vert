#version 410 core

layout (location = 0) in vec2 iPos;
layout (location = 1) in vec3 iColour;

out vec3 oColour;

void main()
{
    gl_Position = vec4(iPos.x, iPos.y, 0, 1);
    oColour = iColour;
}