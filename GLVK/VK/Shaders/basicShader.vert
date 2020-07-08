#version 450

layout (binding = 0) uniform ModelViewProjection
{
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outTexCoord;

void main()
{
    vec4 position = vec4(inPosition, 1.0);
    gl_Position = mvp.projection * mvp.view * mvp.model * position;
    outNormal = inNormal;
    outTexCoord = inTexCoord;
}