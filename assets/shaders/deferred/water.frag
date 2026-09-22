#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColorSpec;
uniform sampler2D sceneColor;
uniform sampler2D noiseTexture;

uniform vec3 viewPos;
uniform mat4 view;
uniform mat4 projection;

uniform float time;
uniform float waterHeight;
uniform vec4 waterColor;
uniform float waveSpeed;
uniform float waveStrength;
uniform float tiling;
uniform float reflectivity;

// SSR Parameters
uniform bool ssrEnabled;
uniform int maxSteps;
uniform float stepSize;
uniform float thickness;

vec2 RayCast(vec3 dir, inout vec3 hitPos, inout float dDepth)
{
    vec3 rayPos = hitPos;
    
    for(int i = 0; i < maxSteps; i++)
    {
        rayPos += dir * stepSize;
        
        vec4 projectedCoord = projection * view * vec4(rayPos, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
        
        if (projectedCoord.x < 0.0 || projectedCoord.x > 1.0 || projectedCoord.y < 0.0 || projectedCoord.y > 1.0)
            return vec2(-1.0);

        float depth = texture(gPosition, projectedCoord.xy).w; // Assuming w stores depth or use position.z
        // If not using w, we can compare world space positions
        vec3 worldPos = texture(gPosition, projectedCoord.xy).xyz;
        
        // Convert to view space for more accurate comparison if needed, 
        // but here we use world space distance from camera
        float currentDepth = length(rayPos - viewPos);
        float sampleDepth = length(worldPos - viewPos);
        
        if (currentDepth >= sampleDepth && currentDepth - sampleDepth < thickness)
        {
            return projectedCoord.xy;
        }
    }
    
    return vec2(-1.0);
}

void main()
{
    vec3 worldPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    
    // Ray from camera through this pixel
    vec3 rayDir = normalize(worldPos - viewPos);
    
    // Intersection of ray with water plane (y = waterHeight)
    float t = (waterHeight - viewPos.y) / rayDir.y;
    
    float distToGeom = length(worldPos - viewPos);
    if (length(worldPos) <= 0.0001) distToGeom = 1000000.0; // Assume very far for sky
    
    // If looking up or geom is in front of water, just return scene color
    if (t <= 0.0 || t > distToGeom)
    {
        FragColor = texture(sceneColor, TexCoords);
        return;
    }
    
    vec3 waterPos = viewPos + rayDir * t;
    
    // Waves/Noise for Normal
    vec2 noiseCoords = waterPos.xz * tiling + time * waveSpeed;
    vec3 noise = texture(noiseTexture, noiseCoords).rgb * 2.0 - 1.0;
    vec3 waterNormal = normalize(vec3(noise.x * waveStrength, 1.0, noise.y * waveStrength));
    
    // Reflected vector
    vec3 viewDir = normalize(waterPos - viewPos);
    vec3 reflectDir = reflect(viewDir, waterNormal);
    
    vec3 reflectionColor = vec3(0.0);
    if (ssrEnabled)
    {
        vec3 hitPos = waterPos;
        float dDepth;
        vec2 coords = RayCast(reflectDir, hitPos, dDepth);
        
        if (coords != vec2(-1.0))
        {
            reflectionColor = texture(sceneColor, coords).rgb;
        }
        else
        {
            // Fallback to simpler look
            reflectionColor = waterColor.rgb * 0.5; 
        }
    }
    
    // Fresnel
    float fresnel = pow(1.0 - max(dot(waterNormal, -viewDir), 0.0), 5.0);
    
    vec3 colorUnderWater = texture(sceneColor, TexCoords).rgb;
    vec3 waterResult = mix(waterColor.rgb, reflectionColor, reflectivity + fresnel * (1.0 - reflectivity));
    
    // Manual blending in shader to avoid empty background issues
    vec3 finalColor = mix(colorUnderWater, waterResult, waterColor.a);
    
    FragColor = vec4(finalColor, 1.0);
}
