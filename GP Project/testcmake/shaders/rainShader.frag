//lightCube.frag


#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D rainTexture; // Sampler for the sun's texture

void main()
{
    FragColor = texture(rainTexture, TexCoords); // Apply the texture
}
