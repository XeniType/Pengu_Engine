#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // The local position of the cube vertex acts perfectly as a 3D texture coordinate
    TexCoords = aPos; 
    
    vec4 pos = projection * view * vec4(aPos, 1.0);
    
    // Force the depth to be exactly 1.0 (the maximum depth)
    gl_Position = pos.xyww; 
}