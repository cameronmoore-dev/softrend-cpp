#version 410 core

in vec3 oColour;

out vec4 FragColor;

void main()
{
    FragColor = vec4(oColour, 1.0);
}