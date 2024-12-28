#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 rainColor;

void main()
{
    FragColor = vec4(rainColor, 0.5); // 0.5 alpha for transparency
}
