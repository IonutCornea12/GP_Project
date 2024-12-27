//shaderStart.frag

#version 410 core
in vec3  fNormal;
in vec4  fPosEye;
in vec4  fPosWorld;

in vec4  fragPosLightSpace;
in vec2  fTexCoords;

out vec4 fColor;

// Uniform toggles
uniform bool enableFog;
uniform bool nightMode;
uniform bool shadowsEnabled;
uniform bool showDepthMap;

// Directional light
uniform vec3 lightDir;
uniform vec3 lightColor;

// 6 point light positions (in EYE space) + color
uniform vec3 pointLightPositions[6];
uniform vec3 pointLightColor;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

// Global lighting params
float ambientStrength  = 0.3f;
float specularStrength = 0.2f;
float shininess        = 32.0f;

// ------------------------------------------------
// 1) Attenuation for point lights
float computeAttenuation(vec3 lightPos, vec3 fragPos)
{
    float distance = length(lightPos - fragPos);
    float constant = 1.0;
    float linear   = 0.03;
    float quadratic = 0.001;
    
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

    // Soft cutoff
    float maxRange = 30.0;
    attenuation *= clamp(1.0 - (distance / maxRange), 0.0, 1.0);

    return attenuation;
}


// ------------------------------------------------
// 3) Fog factor

float computeFog(vec4 fragPosEye)
{
    float fogDensity = nightMode ? 0.04 : 0.02;
    float dist       = length(fragPosEye.xyz);
    float fogFactor  = exp(-pow(dist * fogDensity, 2.0));
    return clamp(fogFactor, 0.0, 1.0);
}


float calculateShadow(vec4 fragPosLightSpace, sampler2D shadowMap) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // Perspective divide
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0, 1] range

    float closestDepth = texture(shadowMap, projCoords.xy).r; // Closest depth in shadow map
    float currentDepth = projCoords.z; // Depth of current fragment
    float bias = max(0.005 * (1.0 - dot(normalize(fNormal), normalize(lightDir))), 0.005); // Avoid acne

    return currentDepth > closestDepth + bias ? 1.0 : 0.0; // Return shadow factor
}
// ------------------------------------------------
void main()
{
    // Base color from the diffuse texture
    vec3 baseColor = texture(diffuseTexture, fTexCoords).rgb;
    
    // Normal and View direction in Eye space
    vec3 normal = normalize(fNormal);
    vec3 viewDir = normalize(-fPosEye.xyz);
    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    //float shadow = 0.0;
    //if(shadowsEnabled) {
    float shadow = calculateShadow(fragPosLightSpace, shadowMap);
    //}
    // -------------------------------
    // A) Directional / Ambient Light
    // -------------------------------
    vec3 directionalLight = vec3(0.0);
    
    if (!nightMode)
    {
        // Daytime ambient + directional
        float ambientAdjustment = 1.0; // Full ambient day
        vec3 ambient   = ambientAdjustment * ambientStrength * lightColor;
        
        // We assume lightDir is also in EYE space
        vec3 lightDirNorm = normalize(lightDir);
        float diff        = max(dot(normal, lightDirNorm), 0.0);
        vec3 diffuse      = diff * lightColor;
        
        // Specular
        vec3 reflectDir   = reflect(-lightDirNorm, normal);
        float spec        = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular     = specularStrength * spec * lightColor;
        
        //directionalLight  = ambient + diffuse + specular;
        directionalLight = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
    }
    else
    {
        float ambientAdjustment = 0.4;
        directionalLight = ambientAdjustment * ambientStrength * lightColor;
    }
    
    // -------------------------------
    // B) Point Lights (Night Only)
    // -------------------------------
    vec3 pointLightingSum = vec3(0.0);
    
    if (nightMode) {
        // 1) Add a global ambient for all objects, e.g. 0.2
        vec3 globalNightAmbient = directionalLight;
        pointLightingSum += globalNightAmbient;
        
        // 2) Then add each lamp
        for (int i = 0; i < 6; i++) {
            vec3 lampDir = normalize(pointLightPositions[i] - fPosWorld.xyz);
            float lampDiff = max(dot(normal, lampDir), 0.0);
            vec3 lampDiffuse = lampDiff * pointLightColor;
            
            vec3 reflectDir = reflect(-lampDir, normal);
            float lampSpec  = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            vec3 lampSpecular = specularStrength * lampSpec * pointLightColor;
            
            float attenuation = computeAttenuation(pointLightPositions[i], fPosWorld.xyz);
            
            // local lamp ambient
            vec3 lampAmbient = 0.1 * pointLightColor;
            
            // combine
            vec3 lampResult = attenuation * (lampAmbient + lampDiffuse + lampSpecular);
            pointLightingSum += lampResult;
        }
    }
   
    //  vec3 shadowedDirectionalLight = (1.0 - shadow) * directionalLight;
    
    vec3 totalLighting = directionalLight + pointLightingSum;

    vec3 finalColor = directionalLight * baseColor;
    
    // -------------------------------
    // D) Fog
    // -------------------------------
    if (enableFog)
    {
        float fogFactor = computeFog(fPosEye);
        vec4 fogColor   = nightMode
        ? vec4(0.1, 0.1, 0.2, 1.0)
        : vec4(0.5, 0.5, 0.5, 1.0);
        
        finalColor = mix(fogColor.rgb, finalColor, fogFactor);
    }

    // Output
    fColor = vec4(finalColor, 1.0);
}
