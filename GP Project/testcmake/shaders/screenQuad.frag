#version 410 core

in vec2 fTexCoords;
out vec4 fColor;

uniform sampler2D depthMap;

void main()
{
    float depthValue = texture(depthMap, fTexCoords).r;

    fColor = vec4(vec3(depthValue), 1.0);
}
