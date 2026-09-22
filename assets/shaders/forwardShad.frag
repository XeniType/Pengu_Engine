#version 460 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;
in mat3 TBN;

uniform vec3 viewPos;

// ─── Material ─────────────────────────────────────────────────────────────────
uniform bool      u_hasAlbedoMap;
uniform vec4      u_albedoColor;
uniform sampler2D u_albedoMap;

uniform bool      u_hasNormalMap;
uniform sampler2D u_normalMap;

// Metallic and Roughness removed for Blinn-Phong
// AO kept in case you still want ambient masking
uniform bool      u_hasAOMap;
uniform sampler2D u_aoMap;

uniform sampler2D DirShadowMap;
uniform sampler2D SpotShadowMap;
uniform samplerCube PointShadowMap;

uniform bool u_isFirstPass;

// ─── Light ────────────────────────────────────────────────────────────────────
struct Light {
    int   light_type;
    vec3  color;
    float intensity;

    vec3  position;
    vec3  direction;

    float ambientStrength;
    float specularStrength;
    float shininess;

    float radius;
    float cutoff;
    float outerCutoff;

    float constant;
    float linear;
    float quadratic; 

    float shadowSoftness;
};

uniform Light light;

// Pre-computed offset directions for point shadow PCF sampling 
// (Replacing the expensive sin/cos loop)
const vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

// ─── Helper Functions ─────────────────────────────────────────────────────────

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(u_normalMap, TexCoords).xyz * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}

// ─── Shadow ───────────────────────────────────────────────────────────────────

float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap, vec3 normal, vec3 lightDir)
{
    if (fragPosLightSpace.w <= 0.0) 
        return 0.0;

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    if (light.light_type == 0) bias *= 0.5; 
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    int pcfRange = int(light.shadowSoftness);
    float samples = 0.0;

    for(int x = -pcfRange; x <= pcfRange; ++x)
    {
        for(int y = -pcfRange; y <= pcfRange; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
            samples++;
        }
    }
    return samples > 0.0 ? (shadow / samples) : 0.0;
}

float PointShadowCalculation(vec3 fragPos, vec3 lightPos, samplerCube shadowCube)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float bias = 0.05; 
    
    float shadow = 0.0;
    int samples = 20;
    float offset = 0.1;
    
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(shadowCube, fragToLight + gridSamplingDisk[i] * offset).r;
        closestDepth *= light.radius;
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    return shadow / float(samples);
}

// ─── Main ─────────────────────────────────────────────────────────────────────

void main()
{
    vec3 albedo = u_hasAlbedoMap ? texture(u_albedoMap, TexCoords).rgb : u_albedoColor.rgb;
    float ao = u_hasAOMap ? texture(u_aoMap, TexCoords).r : 1.0;

    vec3 N = u_hasNormalMap ? getNormalFromMap() : normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    
    vec3 L;
    float attenuation = 1.0;
    
    if (light.light_type == 0) // Directional
    {
        L = normalize(-light.direction);
    }
    else if(light.light_type == 1) // Point
    {
        vec3 toLight = light.position - FragPos;
        float distance = length(toLight);
        L = normalize(toLight);
        
        attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        
        if (distance > light.radius && light.radius > 0.0) attenuation = 0.0;
        
        
    }
    else if (light.light_type == 2) // Spot
    {
        vec3 toLight = light.position - FragPos;
        float distance = length(toLight);
        L = normalize(toLight);
        
        attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

        float theta = dot(L, normalize(-light.direction));
        float epsilon = light.cutoff - light.outerCutoff;
        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
        attenuation *= intensity;
    }

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * albedo;

    vec3 H = normalize(V + L);
    float spec = pow(max(dot(N, H), 0.0), light.shininess);
    vec3 specular = light.specularStrength * spec * vec3(1.0);

    float shadow = 0.0;
    if (light.light_type == 0) // Directional
    {
        shadow = ShadowCalculation(FragPosLightSpace, DirShadowMap, N, L);
    }
    else if (light.light_type == 1) // Point light
    {
        shadow = PointShadowCalculation(FragPos, light.position, PointShadowMap);
    }
    else if (light.light_type == 2) // Spot light
    {
        shadow = ShadowCalculation(FragPosLightSpace, SpotShadowMap, N, L);
    }

    vec3 radiance = light.color * light.intensity * attenuation;
    vec3 Lo = (diffuse + specular) * radiance * (1.0 - shadow);

    vec3 ambient = vec3(0.0);
    if (u_isFirstPass)
    {
        ambient = light.ambientStrength * (light.color * light.intensity) * albedo * ao * attenuation;
    }
    
    vec3 color = ambient + Lo;

    FragColor = vec4(color, 1.0);
}