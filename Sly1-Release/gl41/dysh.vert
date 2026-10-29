#version 430 core

layout (location = 0) in vec3  vertex;
layout (location = 1) in vec3  normal;
layout (location = 2) in vec4  color;
layout (location = 3) in vec2  uv;
layout (location = 4) in uvec4 boneIndices;
layout (location = 5) in vec4  boneWeights;

uniform mat4 model;
uniform mat4 matWorldToClip; 
uniform int fSkin;

// Bone matrices as a buffer texture over GLOBSET::boneMatrixSSBO: one
// column-major mat4 per bone, one column per RGBA32F texel. The texture unit
// is set from C++ (kSamplerUnits in glshaders.h).
uniform samplerBuffer boneMatrixTex;

mat4 BoneMatrix(uint bone)
{
    int texel = int(bone) * 4;
    return mat4(texelFetch(boneMatrixTex, texel),
                texelFetch(boneMatrixTex, texel + 1),
                texelFetch(boneMatrixTex, texel + 2),
                texelFetch(boneMatrixTex, texel + 3));
}

layout(location = 0) out vec4 vertexColor;

void main()
{
    vertexColor = color;

    vec4 positionLocal = vec4(vertex, 1.0);

    if (fSkin != 0)
    {
        mat4 matSkin = mat4(0.0);
        float totalWeight = 0.0;
        int boneCount = textureSize(boneMatrixTex) / 4;

        if (boneWeights.x != 0.0 && boneIndices.x < uint(boneCount))
        {
            matSkin += BoneMatrix(boneIndices.x) * boneWeights.x;
            totalWeight += boneWeights.x;
        }

        if (boneWeights.y != 0.0 && boneIndices.y < uint(boneCount))
        {
            matSkin += BoneMatrix(boneIndices.y) * boneWeights.y;
            totalWeight += boneWeights.y;
        }

        if (boneWeights.z != 0.0 && boneIndices.z < uint(boneCount))
        {
            matSkin += BoneMatrix(boneIndices.z) * boneWeights.z;
            totalWeight += boneWeights.z;
        }

        if (boneWeights.w != 0.0 && boneIndices.w < uint(boneCount))
        {
            matSkin += BoneMatrix(boneIndices.w) * boneWeights.w;
            totalWeight += boneWeights.w;
        }

        if (totalWeight > 0.0)
            positionLocal = matSkin * positionLocal;
    }

    gl_Position = matWorldToClip * model * positionLocal;
}
