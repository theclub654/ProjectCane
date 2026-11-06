#version 430 core
#ifdef DEKO3D
invariant gl_Position;
precise gl_Position;
#endif

layout (location = 0) in vec3  vertex;
layout (location = 1) in vec3  normal;
layout (location = 2) in vec4  color;
layout (location = 3) in vec2  uv;
layout (location = 4) in uvec4 boneIndices;
layout (location = 5) in vec4  boneWeights;

uniform mat4 model;
uniform mat4 matWorldToClip; 
uniform int fSkin;

layout(std430, binding = 9) readonly buffer BONEBLK
{
    mat4 boneMatrices[];
};

layout(location = 0) out vec4 vertexColor;

#ifdef DEKO3D
out float vertexAlpha;
flat out float triangleAlpha;
#endif

void main()
{
    vertexColor = color;
#ifdef DEKO3D
    vertexAlpha = color.a;
    triangleAlpha = color.a;
#endif

    vec4 positionLocal = vec4(vertex, 1.0);

    if (fSkin != 0)
    {
        mat4 matSkin = mat4(0.0);
        float totalWeight = 0.0;
        int boneCount = boneMatrices.length();

        if (boneWeights.x != 0.0 && boneIndices.x < uint(boneCount))
        {
            matSkin += boneMatrices[boneIndices.x] * boneWeights.x;
            totalWeight += boneWeights.x;
        }

        if (boneWeights.y != 0.0 && boneIndices.y < uint(boneCount))
        {
            matSkin += boneMatrices[boneIndices.y] * boneWeights.y;
            totalWeight += boneWeights.y;
        }

        if (boneWeights.z != 0.0 && boneIndices.z < uint(boneCount))
        {
            matSkin += boneMatrices[boneIndices.z] * boneWeights.z;
            totalWeight += boneWeights.z;
        }

        if (boneWeights.w != 0.0 && boneIndices.w < uint(boneCount))
        {
            matSkin += boneMatrices[boneIndices.w] * boneWeights.w;
            totalWeight += boneWeights.w;
        }

        if (totalWeight > 0.0)
            positionLocal = matSkin * positionLocal;
    }

#ifdef DEKO3D
    vec4 worldPosition = model * positionLocal;
    gl_Position = matWorldToClip * worldPosition;
#else
    gl_Position = matWorldToClip * model * positionLocal;
#endif
}
