#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTexCoords;

out vec2 fTexCoords;

void main()
{
    fTexCoords = vTexCoords;
    gl_Position = vec4(vPosition, 1.0f);
}
