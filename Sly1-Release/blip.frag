#version 430 core

in vec2 fragUv;
in vec4 fragColor;

uniform sampler2D diffuseMap;
// The PS2 BLIP packet is replayed as two complementary alpha-test passes:
// solid texels write Z first, then translucent fringe texels blend without Z.
uniform int alphaPass;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 texel = texture(diffuseMap, fragUv);
    float alpha = texel.a * fragColor.a;

    if (alpha <= 0.001)
        discard;

    const float alphaRef = 128.0 / 255.0;
    if (alphaPass == 1 && alpha < alphaRef)
        discard;
    if (alphaPass == 2 && alpha >= alphaRef)
        discard;

    // DrawSw uses SRC_ALPHA, ONE for the BLIP pass, so RGB remains straight
    // rather than premultiplied.
    outColor = vec4(texel.rgb * fragColor.rgb, alpha);
}
