#version 330 core

in vec2 v_texcoord;

uniform sampler2D u_fontTex;
uniform vec4 textColor;

out vec4 fragColor;

void main()
{
    vec4 texColor = texture(u_fontTex, v_texcoord);
    fragColor = texColor * textColor;
}