#version 450

layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec4 fragPos;

layout (location = 0) out vec4 fragColor;

layout (binding = 1) uniform DirectionalLight
{
    vec4 diffuse;
    vec3 light_direction;
    float ambient_intensity;
    float specular_intensity;
} direction_light;

void main()
{
    // Ambient
    vec4 ambient = direction_light.diffuse * direction_light.ambient_intensity;

    // Diffuse Light
    vec4 light_direction = normalize(vec4(-direction_light.light_direction, 1.0));
    vec4 normal = normalize(inNormal);
    float intensity = max(dot(normal, light_direction), 0.0);
    vec4 diffuse = direction_light.diffuse * intensity;

    fragColor = (ambient + diffuse) * vec4(1.0, 1.0, 0.0, 1.0);
}