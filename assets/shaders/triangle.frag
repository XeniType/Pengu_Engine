#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

void main()
{
    FragColor = vec4(TexCoord, 0.25f, 1.0);
}