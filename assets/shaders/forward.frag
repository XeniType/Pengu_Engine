#version 460 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

// ─── Camera ───────────────────────────────────────────────────────────────────
uniform vec3 viewPos;

// ─── Material ─────────────────────────────────────────────────────────────────
uniform bool      u_hasAlbedoMap;
uniform vec4      u_albedoColor;
uniform sampler2D u_albedoMap;

uniform bool      u_hasRoughnessMap;
uniform sampler2D u_roughnessMap;
uniform float     u_roughness;

// ─── Light ────────────────────────────────────────────────────────────────────
struct Light {
    int   light_type;       // 0 = Directional, 1 = Point, 2 = Spot
    vec3  color;

    vec3  position;
    vec3  direction;

    float ambientStrength;
    float specularStrength;
    float shininess;

    float radius;
    float cutoff;           // inner cone (cos angle)
    float outerCutoff;      // outer cone (cos angle)

    float constant;
    float linear;
    float quadratic;
};

uniform Light light;

// ─── Helpers ──────────────────────────────────────────────────────────────────
vec3 sampleDiffuse()
{
    return u_hasAlbedoMap
        ? texture(u_albedoMap, TexCoords).rgb
        : u_albedoColor.rgb;
}

vec3 sampleSpecular()
{
    return u_hasRoughnessMap
        ? texture(u_roughnessMap, TexCoords).rgb
        : vec3(u_roughness);
}

// ─── Directional light ────────────────────────────────────────────────────────
vec3 Calculate_Dir(vec3 matDiff, vec3 matSpec)
{
    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    vec3 viewDir  = normalize(viewPos - FragPos);
    vec3 halfway  = normalize(lightDir + viewDir);

    // Ambient
    vec3 ambient  = light.ambientStrength * light.color * matDiff;

    // Diffuse
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = diff * light.color * matDiff;

    // Specular (Blinn-Phong)
    float spec    = pow(max(dot(norm, halfway), 0.0), light.shininess);
    vec3 specular = light.specularStrength * spec * light.color * matSpec;

    return ambient + diffuse + specular;
}

// ─── Point light ──────────────────────────────────────────────────────────────
vec3 Calculate_Point(vec3 matDiff, vec3 matSpec)
{
    vec3  toLight  = light.position - FragPos;
    float distance = length(toLight);

    // Hard radius cutoff
    if (distance > light.radius)
        return vec3(0.0);

    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(toLight);
    vec3 viewDir  = normalize(viewPos - FragPos);
    vec3 halfway  = normalize(lightDir + viewDir);

    // Attenuation
    float atten   = 1.0 / (light.constant
                  + light.linear    * distance
                  + light.quadratic * distance * distance);

    // Ambient
    vec3 ambient  = light.ambientStrength * light.color * matDiff * atten;

    // Diffuse
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = diff * light.color * matDiff * atten;

    // Specular (Blinn-Phong)
    float spec    = pow(max(dot(norm, halfway), 0.0), light.shininess);
    vec3 specular = light.specularStrength * spec * light.color * matSpec * atten;

    return ambient + diffuse + specular;
}

// ─── Spot light ───────────────────────────────────────────────────────────────
vec3 Calculate_Spot(vec3 matDiff, vec3 matSpec)
{
    vec3  toLight  = light.position - FragPos;
    float distance = length(toLight);

    // Hard radius cutoff
    if (distance > light.radius)
        return vec3(0.0);

    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(toLight);
    vec3 viewDir  = normalize(viewPos - FragPos);
    vec3 halfway  = normalize(lightDir + viewDir);

    // Cone intensity — smooth edge between inner and outer cutoff
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutoff - light.outerCutoff;
    float coneIntensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

    // Return nothing if outside the outer cone
    if (coneIntensity <= 0.0)
        return vec3(0.0);

    // Attenuation
    float atten   = 1.0 / (light.constant
                  + light.linear    * distance
                  + light.quadratic * distance * distance);

    // Ambient (unaffected by cone — keeps scene from going fully black)
    vec3 ambient  = light.ambientStrength * light.color * matDiff * atten;

    // Diffuse
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = diff * light.color * matDiff * atten * coneIntensity;

    // Specular (Blinn-Phong)
    float spec    = pow(max(dot(norm, halfway), 0.0), light.shininess);
    vec3 specular = light.specularStrength * spec * light.color * matSpec * atten * coneIntensity;

    return ambient + diffuse + specular;
}

// ─── Main ─────────────────────────────────────────────────────────────────────
void main()
{
    vec3 matDiff = sampleDiffuse();
    vec3 matSpec = sampleSpecular();

    vec3 result = vec3(0.0);

    if      (light.light_type == 0) result = Calculate_Dir  (matDiff, matSpec);
    else if (light.light_type == 1) result = Calculate_Point(matDiff, matSpec);
    else if (light.light_type == 2) result = Calculate_Spot (matDiff, matSpec);

    FragColor = vec4(result, 1.0);
}
