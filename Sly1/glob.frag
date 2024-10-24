#version 330 core

#define TWPS_Shadow 0
#define TWPS_ShadowMidtone 1
#define TWPS_ShadowMidtoneSaturate 2

uniform sampler2D shadowTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D saturateTexture;

in vec4 vertexColor;
in vec2 texcoord;

uniform int fThreeWay;

in vec4 ambient;
in vec4 saturate;
in vec4 lit;

void main()
{
    int twps = 2;
    vec4 pixel = vec4(0.0);

    if (fThreeWay == 1)
    {
        // Draw Threeway
        vec4 shadowTexel   = texture(shadowTexture,   texcoord);
        vec4 diffuseTexel  = texture(diffuseTexture,  texcoord);
        vec4 saturateTexel = texture(saturateTexture, texcoord);
        
        switch (twps)
        {
            case TWPS_Shadow:
            pixel.r = shadowTexel.r * vertexColor.r;
            pixel.g = shadowTexel.g * vertexColor.g;
            pixel.b = shadowTexel.b * vertexColor.b;
            pixel.a = vertexColor.a;
            break;

            case TWPS_ShadowMidtone:
            pixel.r = shadowTexel.r * vertexColor.r;
            pixel.g = shadowTexel.g * vertexColor.g;
            pixel.b = shadowTexel.b * vertexColor.b;
            pixel.a = vertexColor.a;

            pixel.r += diffuseTexel.r * ambient.r;
            pixel.g += diffuseTexel.g * ambient.g;
            pixel.b += diffuseTexel.b * ambient.b;
            break;
            
            case TWPS_ShadowMidtoneSaturate:
            pixel.r = shadowTexel.r * vertexColor.r;
            pixel.g = shadowTexel.g * vertexColor.g;
            pixel.b = shadowTexel.b * vertexColor.b;
            pixel.a = vertexColor.a;

            pixel.r += diffuseTexel.r * ambient.r;
            pixel.g += diffuseTexel.g * ambient.g;
            pixel.b += diffuseTexel.b * ambient.b;

            pixel.r += saturateTexel.r * saturate.r;
            pixel.g += saturateTexel.g * saturate.g;
            pixel.b += saturateTexel.b * saturate.b;
            break;
        }
    }

    else
    {
        vec4 diffuseTexel  = texture(diffuseTexture,   texcoord);
        pixel.r = diffuseTexel.r * vertexColor.r;
        pixel.g = diffuseTexel.g * vertexColor.g;
        pixel.b = diffuseTexel.b * vertexColor.b;
        pixel.a = diffuseTexel.a;
    }

    gl_FragColor = pixel;
}