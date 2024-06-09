#version 330 core

in vec3 aNormal;
in vec4 aVertexColor;
in vec2 aTexcoord;

uniform sampler2D ambientTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D saturateTexture;

uniform int isAmbient;
uniform int isDiffuse;
uniform int isSaturate;

uniform int fThreeWay;

void main()
{
    vec4 texel = vec4(1.0);

    if (fThreeWay == 1)
    {
        vec4 ambientTexel   = texture2D(ambientTexture,   aTexcoord);
        texel *= ambientTexel;

        vec4 diffuseTexel   = texture2D(diffuseTexture,   aTexcoord);
        texel *= diffuseTexel;

        vec4 saturateTexel = texture2D(saturateTexture,   aTexcoord);
        texel *= saturateTexel;
    }

    else
    {
        vec4 ambientTexel   = texture2D(ambientTexture,   aTexcoord);
        texel *= ambientTexel;
    }

    if (texel.a < 0.1)
       discard;
    
    gl_FragColor = texel * aVertexColor;
}