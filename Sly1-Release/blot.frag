#version 430 core
layout(location = 0) in vec2 v_texcoord;
layout(location = 1) in vec4 v_color;

uniform sampler2D u_fontTex;
uniform vec4 blotColor;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec4 texColor = texture(u_fontTex, v_texcoord);
    fragColor = texColor * blotColor * v_color;
}
