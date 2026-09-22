#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

vec3 WorldPos;
vec3 Normal;

uniform vec3 viewPos;
uniform mat4 view;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gAlbedoAO;
layout(binding = 7) uniform sampler2D gPBR;

layout(binding = 3) uniform sampler2D DirShadowMapCasc1;
layout(binding = 4) uniform sampler2D DirShadowMapCasc2;
layout(binding = 5) uniform sampler2D DirShadowMapCasc3;

layout(binding = 6) uniform sampler2D SpotShadowMap;
layout(binding = 8) uniform samplerCube PointShadowMap;

uniform vec3 fogColor;
uniform float fogDensity;

uniform mat4 lightSpaceMatrices[3];
uniform float cascadeDepths[3];

// Shadow map uniforms
uniform mat4 lightSpaceMatrix;

struct Light {
    int   light_type;
    vec3  color;

    vec3  position;
    vec3  direction;

    float ambientStrength;
    float specularStrength;
    float shininess;

    float radius;
    float cutoff;
    float outerCutoff;
    float intensity;

    float constant;
    float linear;
    float quadratic; 

    float shadowSoftness;
};

uniform Light light;

const float PI = 3.14159265359;

const vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

// ─── PBR Functions ────────────────────────────────────────────────────────────

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ─── Shadow Calculations ──────────────────────────────────────────────────────

float CalculateDirShadow(vec3 fragPos, vec3 normal, vec3 lightDir) 
{
    vec4 fragPosViewSpace = view * vec4(fragPos, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < 3; ++i)
    {
        if (depthValue < cascadeDepths[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1) layer = 2;

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0) return 0.0; 

    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    
    float shadow = 0.0;
    vec2 texelSize;
    float currentDepth = projCoords.z;

    if (layer == 0) {
        texelSize = 1.0 / textureSize(DirShadowMapCasc1, 0);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(DirShadowMapCasc1, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
    } else if (layer == 1) {
        texelSize = 1.0 / textureSize(DirShadowMapCasc2, 0);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(DirShadowMapCasc2, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
    } else {
        texelSize = 1.0 / textureSize(DirShadowMapCasc3, 0);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(DirShadowMapCasc3, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
    }
    
    return shadow / 9.0;
}

float CalculateSpotShadow(vec3 fragPos, vec3 normal, vec3 lightDir) 
{
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 
    
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0 || 
       projCoords.z > 1.0) {
        return 0.0; 
    }

    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(SpotShadowMap, 0);
    float currentDepth = projCoords.z;
    
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(SpotShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}

float CalculatePointShadow(vec3 fragPos, vec3 lightPos)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float bias = 0.05; 
    
    float shadow = 0.0;
    int samples = 20;
    float offset = 0.1;
    
    for(int i = 0; i < samples; ++i)
    {
        // Sample the cubemap using the direction vector
        float closestDepth = texture(PointShadowMap, fragToLight + gridSamplingDisk[i] * offset).r;
        
        // Un-normalize the depth using the light's radius (which acts as the far_plane)
        closestDepth *= light.radius; 
        
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    return shadow / float(samples);
}

// ─── Main ─────────────────────────────────────────────────────────────────────

void main()
{   
    WorldPos = texture(gPosition, TexCoords).rgb;
    Normal = texture(gNormal, TexCoords).rgb;
    vec4 albedoAO = texture(gAlbedoAO, TexCoords);
    vec3 albedo = pow(albedoAO.rgb, vec3(2.2));
    float ao = albedoAO.a;
    vec2 pbr = texture(gPBR, TexCoords).rg;
    float roughness = pbr.r;
    float metallic = pbr.g;

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    vec3 L;
    float attenuation = 1.0;
    float shadow = 0.0;
    int debugCascadeLayer = -1;
    
    if (light.light_type == 0) // Directional
    {
        L = normalize(-light.direction);
        shadow = CalculateDirShadow(WorldPos, N, L);
    }
    else // Point or Spot
    {
        vec3 toLight = light.position - WorldPos;
        float distance = length(toLight);
        L = normalize(toLight);
        attenuation = 1.0 / (distance * distance); // Inverse square falloff for PBR
        
        if (light.light_type == 1)
        {
            shadow = CalculatePointShadow(WorldPos, light.position);
            if (distance > light.radius && light.radius > 0.0) attenuation = 0.0;
        }
        else if (light.light_type == 2) // Spot
        {
            float theta = dot(L, normalize(-light.direction));
            float epsilon = light.cutoff - light.outerCutoff;
            float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
            attenuation *= intensity;
            shadow = CalculateSpotShadow(WorldPos, N, L);
        }
        
    }

    // calculate per-light radiance
    vec3 H = normalize(V + L);
    vec3 radiance = light.color * attenuation * light.intensity;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
       
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  

    float NdotL = max(dot(N, L), 0.0);        

    Lo += (kD * albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);

    // Ambient
    vec3 ambient = light.ambientStrength * light.color * albedo * ao;
    
    vec3 result = ambient + Lo;

    // Fog
    float dist = length(viewPos - WorldPos);
    float fogFactor = exp(-pow((dist * fogDensity), 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    vec3 finalColor = mix(fogColor, result, fogFactor);

    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0/2.2));

    FragColor = vec4(finalColor, 1.0);
}
