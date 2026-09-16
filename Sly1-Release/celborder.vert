#version 430 core

#define MAX_POSES 64

layout(std140, binding = 0) uniform CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

layout(std140, binding = 1) uniform ROCEL
{
    mat4 model;
    vec4 celRgba;
    float uAlphaCelBorder;
} cbp;

layout(std430, binding = 8) readonly buffer EDGEBUFFER
{
    vec4 edges[];
};

layout(std430, binding = 9) readonly buffer BONEBLK
{
    mat4 boneMatrices[];
};

layout(std430, binding = 10) readonly buffer EDGEBONEINDICES
{
    uvec4 edgeBoneIndices[];
};

layout(std430, binding = 11) readonly buffer EDGEBONEWEIGHTS
{
    vec4 edgeBoneWeights[];
};

layout(std430, binding = 12) readonly buffer EDGEPOSEPOSBLK
{
    vec4 edgePoseDpos[];
};

uniform int fSkin;
uniform int fPose;
uniform int poseCount;
uniform float poseWeights[MAX_POSES];

void CullVertex();
float Cross2D(vec2 a, vec2 b);
vec3 GetEdgePosition(int pointIndex);

void main()
{
    int edgeID = gl_VertexID >> 1;
    bool first = (gl_VertexID & 1) == 0;
    int pointBase = edgeID * 4;

    vec3 E0 = GetEdgePosition(pointBase + 0);
    vec3 E1 = GetEdgePosition(pointBase + 1);
    vec3 OA = GetEdgePosition(pointBase + 2);
    vec3 OB = GetEdgePosition(pointBase + 3);

    mat4 mvp = cm.matWorldToClip * cbp.model;

    vec4 A = mvp * vec4(E0, 1.0);
    vec4 B = mvp * vec4(E1, 1.0);
    vec4 C = mvp * vec4(OA, 1.0);
    vec4 D = mvp * vec4(OB, 1.0);

    if (A.w <= 0.0 || B.w <= 0.0 || C.w <= 0.0 || D.w <= 0.0)
    {
        CullVertex();
        return;
    }

    vec3 a = A.xyz / A.w;
    vec3 b = B.xyz / B.w;
    vec3 c = C.xyz / C.w;
    vec3 d = D.xyz / D.w;

    float cross0 = Cross2D(b.xy - a.xy, c.xy - b.xy);
    float cross1 = Cross2D(d.xy - b.xy, b.xy - a.xy);

    if ((cross0 * cross1) > 0.0)
    {
        CullVertex();
        return;
    }

    gl_Position = first ? A : B;
}

void CullVertex()
{
    gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
}

float Cross2D(vec2 a, vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

vec3 GetEdgePosition(int pointIndex)
{
    vec4 position = edges[pointIndex];

    if (fPose != 0)
    {
        int count = min(poseCount, MAX_POSES);
        int poseBase = pointIndex * poseCount;

        for (int ipose = 0; ipose < count; ++ipose)
        {
            float weight = poseWeights[ipose];

            if (weight != 0.0)
                position.xyz += edgePoseDpos[poseBase + ipose].xyz * weight;
        }
    }

    if (fSkin == 0)
        return position.xyz;

    uvec4 indices = edgeBoneIndices[pointIndex];
    vec4 weights = edgeBoneWeights[pointIndex];
    vec4 localPosition = vec4(position.xyz, 1.0);
    vec4 skinnedPosition = vec4(0.0);
    float totalWeight = 0.0;

    if (weights.x != 0.0)
    {
        skinnedPosition += boneMatrices[indices.x] * localPosition * weights.x;
        totalWeight += weights.x;
    }

    if (weights.y != 0.0)
    {
        skinnedPosition += boneMatrices[indices.y] * localPosition * weights.y;
        totalWeight += weights.y;
    }

    if (weights.z != 0.0)
    {
        skinnedPosition += boneMatrices[indices.z] * localPosition * weights.z;
        totalWeight += weights.z;
    }

    if (weights.w != 0.0)
    {
        skinnedPosition += boneMatrices[indices.w] * localPosition * weights.w;
        totalWeight += weights.w;
    }

    return totalWeight > 0.0 ? skinnedPosition.xyz : position.xyz;
}