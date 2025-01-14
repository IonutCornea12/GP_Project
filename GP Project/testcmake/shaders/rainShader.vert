#version 410 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;

void main()
{
    TexCoords = vTexCoords;
    gl_Position = projection * view * model * vec4(vPos, 1.0);
}
