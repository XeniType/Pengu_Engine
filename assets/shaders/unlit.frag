#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform bool      u_hasAlbedoMap;
uniform vec4      u_albedoColor;
uniform sampler2D u_albedoMap;

void main() {
    vec3 result = u_hasAlbedoMap
        ? texture(u_albedoMap, TexCoords).rgb
        : u_albedoColor.rgb;

    FragColor = vec4(result, 1.0);
}