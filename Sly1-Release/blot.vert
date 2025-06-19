#version 330 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_texcoord;

uniform mat4 u_model;
uniform mat4 u_projection;
uniform vec4 u_uvRect;

out vec2 v_texcoord;

void main()
{
    vec2 uvMin = u_uvRect.xy;
    vec2 uvMax = u_uvRect.zw;

    vec2 uv = mix(uvMin, uvMax, a_texcoord);
    
    v_texcoord = uv;

    gl_Position = u_projection * u_model * vec4(a_position, 0.0, 1.0);
}