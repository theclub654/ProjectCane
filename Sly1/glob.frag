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
in vec4 illumination;
in vec4 light;

void main()
{
    int twps = 2;
    vec4 pixel = vec4(0.0);

    if (fThreeWay == 1)
    {
        // Draw Threeway
        vec4 shadow   = texture2D(shadowTexture,   texcoord);
        vec4 diffuse  = texture2D(diffuseTexture,  texcoord);
        vec4 saturate = texture2D(saturateTexture, texcoord);

        switch (twps)
        {
            case TWPS_Shadow:
            pixel += shadow * ambient;

            pixel.a = vertexColor.a;
            break;
            
            case TWPS_ShadowMidtone:
            pixel += shadow  * ambient;
            pixel += diffuse * illumination;

            pixel.a = vertexColor.a;
            break;
            
            case TWPS_ShadowMidtoneSaturate:
            pixel += shadow   * ambient;
            pixel += diffuse  * illumination;
            pixel += saturate * light;
            
            pixel.a = vertexColor.a;
            break;
        }
    }

    else
    {
        // Draws Prelit
        vec4 diffuseTexel  = texture(diffuseTexture, texcoord);
        pixel   = diffuseTexel * vertexColor;
        pixel.a = diffuseTexel.a;
    }

     gl_FragColor = pixel;
}