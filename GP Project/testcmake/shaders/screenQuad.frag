//#version 410 core
//uniform sampler2D shadowMap;
//
//in vec2 TexCoords;          // Receive texture coordinates
//in vec4 fragPosLightSpace;
//
//void main() {
//    
//    
//    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//    projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1] range
//    float shadow = texture(shadowMap, projCoords.xy).r < projCoords.z ? 1.0 : 0.0;
//    // Combine shadow with scene color here
//}

#version 410 core

in vec2 fTexCoords;

out vec4 fColor;

uniform sampler2D depthMap;

void main()
{
    fColor = vec4(vec3(texture(depthMap, fTexCoords).r), 1.0f);
    //fColor = vec4(fTexCoords, 0.0f, 1.0f);
}
