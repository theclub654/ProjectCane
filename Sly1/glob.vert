#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 texcoord;

out vec3 aNormal;
out vec4 aColor;
out vec2 aTexcoord;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec4 result;

vec4 CalculatePointLight(vec3 position);

void main()
{
    aNormal = normal;
    aColor = color;
    aTexcoord = texcoord;

    vec3 lightPos0 = vec3(0.0, 0.0, 0.0);

    result = CalculatePointLight(lightPos0);

    gl_Position = proj * view * model * vec4(pos, 1.0);
}

vec4 CalculatePointLight(vec3 position)
{
    vec4 color = vec4(1.0, 1.0, 1.0, 0.0);

    float ambientStrength = 1.0;
    vec4 ambient = ambientStrength * color;

    return ambient;
}