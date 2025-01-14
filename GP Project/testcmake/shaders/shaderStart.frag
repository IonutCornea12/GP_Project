// shaderStart.frag
#version 410 core
out vec4 fColor;

in vec3  fNormal;
in vec4  fPosEye;
in vec4  fPosWorld;
in vec4  fragPosLightSpace;
in vec2  fTexCoords;

// Uniform toggles
uniform bool enableFog;
uniform bool nightMode;
uniform bool showDepthMap;
uniform bool rainEnabled;
uniform bool flashActive;

// Directional light
uniform vec3 lightDir;
uniform vec3 lightDirWorld;
uniform vec3 lightColor;

// 6 point light positions (in World space) + color
uniform vec3 pointLightPositions[6];
uniform vec3 pointLightColor;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

// Flash effect multiplier
uniform float flashMultiplier;

// Camera position
uniform vec3 viewPos;

// Global lighting params
float ambientStrength  = 0.2f;
float specularStrength = 2.5f;
float shininess        = 16.0f;

// ------------------------------------------------
// 1) Attenuation for point lights
float computeAttenuation(vec3 lightPos, vec3 fragPos)
{
    float distance = length(lightPos - fragPos);
    float constant = 1.0;
    float linear   = 0.09;
    float quadratic = 0.032;
    
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

    // Soft cutoff
    float maxRange = 50.0;
    attenuation *= clamp(1.0 - (distance / maxRange), 0.0, 1.0);

    return attenuation;
}

// ------------------------------------------------
// 3) Fog factor

float computeFog(vec4 fragPosEye)
{
    float fogDensity;
    
    if (rainEnabled) {
        fogDensity = nightMode ? 0.03 : 0.01; // Reduced fog density during rain
    }
    else {
        fogDensity = nightMode ? 0.04 : 0.02;
    }
    
    float dist       = length(fragPosEye.xyz);
    float fogFactor  = exp(-pow(dist * fogDensity, 2.0));
    return clamp(fogFactor, 0.0, 1.0);
}

float calculateShadow(vec4 fragPosLightSpace, sampler2D shadowMap) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // If fragment is outside the shadow map, consider it lit
    if(projCoords.z > 1.0)
        return 0.0;

    // Declare currentDepth and bias
    float currentDepth = projCoords.z;
    // Dynamic bias based on angle between normal and light direction
    float bias = max(0.002 * (1.0 - dot(normalize(fNormal), normalize(lightDir))), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    // 3x3 PCF
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > closestDepth + bias ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main()
{
    // Base color from the diffuse texture
    vec3 baseColor = texture(diffuseTexture, fTexCoords).rgb;
    
    // Normalize normals and view direction
    vec3 normal = normalize(fNormal);
    
    vec3 viewDir = normalize(-fPosEye.xyz);
    // Initialize lighting components
    vec3 ambient = vec3(0.0);
    vec3 directionalLight = vec3(0.0);
    vec3 pointLightingSum = vec3(0.0);
    
    // Calculate shadow factor (disabled if fog is enabled, it's night, or it's raining)
    float shadow = (enableFog || nightMode || rainEnabled) ? 0.0 : calculateShadow(fragPosLightSpace, shadowMap);
    
    // Directional Light Calculation
    if (!nightMode)
    {
        // Daytime ambient + directional
        float ambientAdjustment = 1.0; // Full ambient day
        ambient = ambientAdjustment * ambientStrength * lightColor * flashMultiplier;
        
        // Normalize light direction
        vec3 lightDirNorm = normalize(lightDir);
        float diff = max(dot(normal, lightDirNorm), 0.0);
        vec3 diffuse = diff * lightColor * flashMultiplier;
        vec3 reflectDir = reflect(-lightDirNorm, normal);
        
        // Phong highlight
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * lightColor * flashMultiplier;
        // Apply shadow to diffuse and specular
        directionalLight = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
    }
    else
    {
        // Nighttime ambient
        float ambientAdjustment = 0.1; // Darker ambient light at night
        ambient = ambientAdjustment * ambientStrength * lightColor * flashMultiplier;
        directionalLight = ambient; // No directional light during night
    }
    
    // Point Lights Calculation (only for night mode)
    if (nightMode) {
        // Global ambient contribution from directional light
        vec3 globalNightAmbient = directionalLight;
        pointLightingSum += globalNightAmbient;
        
        // Iterate through each point light
        for (int i = 0; i < 6; i++) {
            // Direction from fragment to light
            vec3 lampDir = normalize(pointLightPositions[i] - fPosWorld.xyz);
            float lampDiff = max(dot(normal, lampDir), 0.0);
            vec3 lampDiffuse = lampDiff * pointLightColor * flashMultiplier;
            
            // Specular component
            vec3 reflectDir = reflect(-lampDir, normal);
            float lampSpec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            vec3 lampSpecular = specularStrength * lampSpec * pointLightColor * flashMultiplier;
            
            // Attenuation
            float attenuation = computeAttenuation(pointLightPositions[i], fPosWorld.xyz);
            
            // Local ambient
            vec3 lampAmbient = 0.1 * pointLightColor * flashMultiplier;
            
            // Combine results
            vec3 lampResult = attenuation * (lampAmbient + lampDiffuse + lampSpecular);
            pointLightingSum += lampResult;
        }
    }
    
    // Total Lighting
    vec3 totalLighting = directionalLight + pointLightingSum;

    // Apply base color
    vec3 finalColor = totalLighting * baseColor;
    
    // Apply Fog if enabled or if rain is active
    if (enableFog || rainEnabled)
    {
        float fogFactor = computeFog(fPosEye);
        vec4 fogColor;
        
        if (rainEnabled)
        {
            fogColor = vec4(0.8, 0.8, 0.8, 1.0);
        }
        else
        {
            fogColor = nightMode
                ? vec4(0.1, 0.1, 0.2, 1.0)
                : vec4(0.5, 0.5, 0.5, 1.0);
        }
        
        finalColor = mix(fogColor.rgb, finalColor, fogFactor);
    }

    // Output final color
    if (flashActive)
    {
        fColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
    else
    {
        // Output final color
        fColor = vec4(finalColor, 1.0);
    }
}
