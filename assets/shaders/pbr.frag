#version 460 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;

// ─── Camera ───────────────────────────────────────────────────────────────────
uniform vec3 viewPos;

// ─── Material ─────────────────────────────────────────────────────────────────
uniform bool      u_hasAlbedoMap;
uniform vec4      u_albedoColor;
uniform sampler2D u_albedoMap;

uniform bool      u_hasNormalMap;
uniform sampler2D u_normalMap;

uniform bool      u_hasRoughnessMap;
uniform sampler2D u_roughnessMap;
uniform float     u_roughness;

uniform bool      u_hasMetallicMap;
uniform sampler2D u_metallicMap;
uniform float     u_metallic;

uniform bool      u_hasAOMap;
uniform sampler2D u_aoMap;

// ─── Light ────────────────────────────────────────────────────────────────────
struct Light {
    int   light_type;       // 0 = Directional, 1 = Point, 2 = Spot
    vec3  color;

    vec3  position;
    vec3  direction;

    float ambientStrength;
    float specularStrength; // Not strictly used in PBR but kept for compatibility
    float shininess;        // Not strictly used in PBR

    float radius;
    float cutoff;           
    float outerCutoff;      

    float constant;
    float linear;
    float quadratic;
};

uniform Light light;

const float PI = 3.14159265359;

// ─── PBR Functions ────────────────────────────────────────────────────────────

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(u_normalMap, TexCoords).xyz * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}

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

// ─── Main ─────────────────────────────────────────────────────────────────────

void main()
{
    vec3 albedo = u_hasAlbedoMap ? pow(texture(u_albedoMap, TexCoords).rgb, vec3(2.2)) : u_albedoColor.rgb;
    float metallic = u_hasMetallicMap ? texture(u_metallicMap, TexCoords).r : u_metallic;
    float roughness = u_hasRoughnessMap ? texture(u_roughnessMap, TexCoords).r : u_roughness;
    float ao = u_hasAOMap ? texture(u_aoMap, TexCoords).r : 1.0;

    vec3 N = u_hasNormalMap ? getNormalFromMap() : normalize(Normal);
    vec3 V = normalize(viewPos - WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    
    // Light calculation
    vec3 L;
    float attenuation = 1.0;
    
    if (light.light_type == 0) // Directional
    {
        L = normalize(-light.direction);
    }
    else // Point or Spot
    {
        vec3 toLight = light.position - WorldPos;
        float distance = length(toLight);
        L = normalize(toLight);
        attenuation = 1.0 / (distance * distance); // Inverse square law for PBR
        
        if (light.light_type == 2) // Spot
        {
            float theta = dot(L, normalize(-light.direction));
            float epsilon = light.cutoff - light.outerCutoff;
            float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
            attenuation *= intensity;
        }
        
        if (distance > light.radius && light.radius > 0.0) attenuation = 0.0;
    }

    // calculate per-light radiance
    vec3 H = normalize(V + L);
    vec3 radiance = light.color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
       
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  

    float NdotL = max(dot(N, L), 0.0);        

    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    // Ambient
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}
