//shaderStart.vert

#version 410 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoords;

out vec3 fNormal;
out vec4 fPosEye;
out vec4 fPosWorld;
out vec2 fTexCoords;
out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform mat3 normalMatrix;

void main()
{
    fPosWorld = model * vec4(vPosition, 1.0f);
    fPosEye = view * fPosWorld;
    fNormal = normalize(normalMatrix * vNormal);
    fTexCoords = vTexCoords;
    fragPosLightSpace = lightSpaceMatrix  * fPosWorld;
    gl_Position = projection * fPosEye;
    
    
}
