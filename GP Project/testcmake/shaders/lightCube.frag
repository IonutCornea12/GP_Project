//lightCube.frag


#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D sunTexture;

void main()
{
    FragColor = texture(sunTexture, TexCoords); // Apply the texture
}
