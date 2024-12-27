//lightCube.vert

#version 410 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoords; // Ensure texture coordinates are included

out vec2 TexCoords; // Pass to fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = vTexCoords; // Pass texture coordinates
    gl_Position = projection * view * model * vec4(vPosition, 1.0);
}
