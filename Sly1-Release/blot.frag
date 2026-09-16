#version 430 core
#extension GL_ARB_bindless_texture : require

in vec2 v_texcoord;
in vec4 v_color;

uniform sampler2D u_fontTex;
uniform vec4 blotColor;

out vec4 fragColor;

void main()
{
    vec4 texColor = texture(u_fontTex, v_texcoord);
    fragColor = texColor * blotColor * v_color;
}