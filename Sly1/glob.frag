#version 330 core

in vec3 aNormal;
in vec4 aVertexColor;
in vec2 aTexcoord;

uniform sampler2D ambientTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D greyscaleTexture;

uniform int isAmbient;
uniform int isDiffuse;
uniform int isGreyScale;

void main()
{
    vec4 texel = vec4(1.0);

    if (isAmbient != 0)
    {
        vec4 ambientTexel   = texture2D(ambientTexture,   aTexcoord);
        texel *= ambientTexel;
    }

    if (isDiffuse != 0)
    {
        vec4 diffuseTexel   = texture2D(diffuseTexture,   aTexcoord);
        texel *= diffuseTexel;
    }

    if (isGreyScale != 0)
    {
        vec4 greyScaleTexel = texture2D(greyscaleTexture, aTexcoord);
        texel *= greyScaleTexel;
    }

    if (texel.a < 0.1)
       discard; 
    
    gl_FragColor = texel * aVertexColor;
}