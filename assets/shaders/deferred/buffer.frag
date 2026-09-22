#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoAO;
layout (location = 3) out vec2 gPBR;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

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

uniform bool DrawNormals;
uniform bool DrawUV;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(u_normalMap, TexCoords).xyz * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}

void main()
{    
    gPosition = FragPos;

    gNormal = u_hasNormalMap ? getNormalFromMap() : normalize(Normal);

    vec3 albedo = u_hasAlbedoMap ? texture(u_albedoMap, TexCoords).rgb : u_albedoColor.rgb;
    float ao = u_hasAOMap ? texture(u_aoMap, TexCoords).r : 1.0;
    
    gAlbedoAO.rgb = albedo;
    gAlbedoAO.a = ao;

    float roughness = u_hasRoughnessMap ? texture(u_roughnessMap, TexCoords).g : u_roughness;
    float metallic = u_hasMetallicMap ? texture(u_metallicMap, TexCoords).b : u_metallic;
    
    gPBR.r = roughness;
    gPBR.g = metallic;

    if(DrawNormals){
        gAlbedoAO.rgb = normalize(Normal);
    }

    if(DrawUV){
        gAlbedoAO.rgb = vec3(TexCoords.xy, 0.0f);
    }
}
