#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoAO;
layout (location = 3) out vec2 gPBR;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;

struct TerrainLayer {
    sampler2D albedoMap;
    sampler2D normalMap;
    sampler2D roughnessMap;
    sampler2D metallicMap;
    sampler2D aoMap;

    bool hasNormalMap;
    bool hasRoughnessMap;
    bool hasMetallicMap;
    bool hasAOMap;

    float tiling;
};

uniform TerrainLayer textureBase;
uniform TerrainLayer layers[4];

uniform sampler2D weightMap; 
uniform float maxTerrainHeight; 
uniform float u_seaLevel;
uniform sampler2D noiseTex;

const float WARP_STRENGTH = 0.05;

uniform bool DrawUV;
uniform bool DrawNormals;

struct SampleResult {
    vec3 albedo;
    vec3 normal;
    float roughness;
    float metallic;
    float ao;
};

vec4 sampleTriplanar(sampler2D tex, vec2 uv, float tiling) {
    // Basic micro/macro blending for terrain textures
    vec4 micro = texture(tex, uv * tiling);
    mat2 rot = mat2(0.707, -0.707, 0.707, 0.707);
    vec4 macro = texture(tex, (rot * uv) * (tiling * 0.137));
    return mix(micro, macro, 0.5);
}

SampleResult sampleLayer(TerrainLayer layer, vec2 uv) {
    SampleResult res;
    res.albedo = sampleTriplanar(layer.albedoMap, uv, layer.tiling).rgb;
    
    if (layer.hasNormalMap) {
        vec3 tangentNormal = sampleTriplanar(layer.normalMap, uv, layer.tiling).xyz * 2.0 - 1.0;
        res.normal = normalize(TBN * tangentNormal);
    } else {
        res.normal = normalize(Normal);
    }

    res.roughness = layer.hasRoughnessMap ? sampleTriplanar(layer.roughnessMap, uv, layer.tiling).r : 0.8;
    res.metallic = layer.hasMetallicMap ? sampleTriplanar(layer.metallicMap, uv, layer.tiling).r : 0.0;
    res.ao = layer.hasAOMap ? sampleTriplanar(layer.aoMap, uv, layer.tiling).r : 1.0;

    return res;
}

void main()
{
    gPosition = WorldPos;

    // Sample the procedural weight map
    vec4 weights = texture(weightMap, TexCoords);
    vec2 worldUV = WorldPos.xz * 0.01;  
    vec2 warpOffset = (texture(noiseTex, worldUV).rg * 2.0 - 1.0) * WARP_STRENGTH;
    vec2 finalUV = worldUV + warpOffset;

    // Sample all layers
    SampleResult base = sampleLayer(textureBase, finalUV);
    SampleResult l0 = sampleLayer(layers[0], finalUV);
    SampleResult l1 = sampleLayer(layers[1], finalUV);
    SampleResult l2 = sampleLayer(layers[2], finalUV);
    SampleResult l3 = sampleLayer(layers[3], finalUV);

    // Blending
    float totalOverlay = weights.r + weights.g + weights.b + weights.a;
    float dirtWeight = clamp(1.0 - totalOverlay, 0.0, 1.0);

    vec3 finalAlbedo = (l0.albedo * weights.r) + 
                       (l1.albedo * weights.g) + 
                       (l2.albedo * weights.b) + 
                       (l3.albedo * weights.a) + 
                       (base.albedo * dirtWeight);

    vec3 finalNormal = (l0.normal * weights.r) + 
                       (l1.normal * weights.g) + 
                       (l2.normal * weights.b) + 
                       (l3.normal * weights.a) + 
                       (base.normal * dirtWeight);

    float finalRoughness = (l0.roughness * weights.r) + 
                           (l1.roughness * weights.g) + 
                           (l2.roughness * weights.b) + 
                           (l3.roughness * weights.a) + 
                           (base.roughness * dirtWeight);

    float finalMetallic = (l0.metallic * weights.r) + 
                          (l1.metallic * weights.g) + 
                          (l2.metallic * weights.b) + 
                          (l3.metallic * weights.a) + 
                          (base.metallic * dirtWeight);

    float finalAO = (l0.ao * weights.r) + 
                    (l1.ao * weights.g) + 
                    (l2.ao * weights.b) + 
                    (l3.ao * weights.a) + 
                    (base.ao * dirtWeight);

    gNormal = normalize(finalNormal);

    if (WorldPos.y < u_seaLevel) {
        float depth = clamp((u_seaLevel - WorldPos.y) / 15.0, 0.0, 1.0);
        finalAlbedo = mix(finalAlbedo, vec3(0.01, 0.05, 0.1), depth * 0.8);
    }

    if(DrawNormals)
    {
        finalAlbedo = normalize(Normal) * 0.5 + 0.5;
    }
    else if(DrawUV)
    {
        finalAlbedo = vec3(TexCoords.xy, 0.0f);
    }

    gAlbedoAO.rgb = finalAlbedo;
    gAlbedoAO.a = finalAO;

    gPBR.r = finalRoughness;
    gPBR.g = finalMetallic;
}
