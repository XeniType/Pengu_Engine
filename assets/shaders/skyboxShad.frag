#version 460 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox; 
uniform vec3 fogColor;

void main()
{    
    vec3 skyColor = texture(skybox, TexCoords).rgb;

    vec3 dir = normalize(TexCoords);

    float horizonMix = smoothstep(0.0, 0.3, abs(dir.y)); 

    vec3 finalColor = mix(fogColor, skyColor, horizonMix);
    
    FragColor = vec4(finalColor, 1.0);
}