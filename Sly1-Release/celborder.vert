#version 430 core
#ifdef DEKO3D
invariant gl_Position;
precise gl_Position;
#define EDGE_POSITION vec4
#else
#define EDGE_POSITION vec3
#endif


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

struct CELPOINT
{
    vec4 position;
    uvec4 boneIndices;
    vec4 boneWeights;
};

layout(std430, binding = 8) readonly buffer EDGEBUFFER
{
    CELPOINT edgePoints[];
};

layout(std430, binding = 9) readonly buffer BONEBLK
{
    mat4 boneMatrices[];
};

layout(std430, binding = 12) readonly buffer EDGEPOSEPOSBLK
{
    vec4 edgePoseDpos[];
};

layout(std430, binding = 14) readonly buffer POSEWEIGHTBLK
{
    float poseWeights[];
};

uniform int fSkin;
uniform int fPose;
uniform int poseCount;

void CullVertex();
float Cross2D(vec2 a, vec2 b);
EDGE_POSITION GetEdgePosition(int pointIndex);

void main()
{
    int edgeID = gl_VertexID >> 1;
    bool first = (gl_VertexID & 1) == 0;
    int pointBase = edgeID * 4;

    if (pointBase < 0 || pointBase + 3 >= edgePoints.length())
    {
        CullVertex();
        return;
    }

#ifdef DEKO3D
    vec4 E0 = cbp.model * GetEdgePosition(pointBase + 0);
    vec4 E1 = cbp.model * GetEdgePosition(pointBase + 1);
    vec4 OA = cbp.model * GetEdgePosition(pointBase + 2);
    vec4 OB = cbp.model * GetEdgePosition(pointBase + 3);
    precise vec4 A = cm.matWorldToClip * E0;
    precise vec4 B = cm.matWorldToClip * E1;
    precise vec4 C = cm.matWorldToClip * OA;
    precise vec4 D = cm.matWorldToClip * OB;
#else
    vec3 E0 = GetEdgePosition(pointBase + 0);
    vec3 E1 = GetEdgePosition(pointBase + 1);
    vec3 OA = GetEdgePosition(pointBase + 2);
    vec3 OB = GetEdgePosition(pointBase + 3);
    mat4 mvp = cm.matWorldToClip * cbp.model;
    vec4 A = mvp * vec4(E0, 1.0);
    vec4 B = mvp * vec4(E1, 1.0);
    vec4 C = mvp * vec4(OA, 1.0);
    vec4 D = mvp * vec4(OB, 1.0);
#endif

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

EDGE_POSITION GetEdgePosition(int pointIndex)
{
    CELPOINT point = edgePoints[pointIndex];
    vec4 position = point.position;

    if (fPose != 0)
    {
        int count = min(max(poseCount, 0), poseWeights.length());
        int poseBase = pointIndex * poseCount;

        for (int ipose = 0; ipose < count; ++ipose)
        {
            float weight = poseWeights[ipose];

            int poseIndex = poseBase + ipose;
            if (weight != 0.0 && poseIndex >= 0 &&
                poseIndex < edgePoseDpos.length())
                position.xyz += edgePoseDpos[poseIndex].xyz * weight;
        }
    }

    if (fSkin == 0)
        return EDGE_POSITION(position);

    if (pointIndex < 0 || pointIndex >= edgePoints.length())
        return EDGE_POSITION(position);

    uvec4 indices = point.boneIndices;
    vec4 weights = point.boneWeights;
    vec4 localPosition = vec4(position.xyz, 1.0);
#ifdef DEKO3D
    mat4 matSkin = mat4(0.0);
#else
    vec4 skinnedPosition = vec4(0.0);
#endif
    float totalWeight = 0.0;
    int boneCount = boneMatrices.length();

    if (weights.x != 0.0 && indices.x < uint(boneCount))
    {
#ifdef DEKO3D
        matSkin += boneMatrices[indices.x] * weights.x;
#else
        skinnedPosition += boneMatrices[indices.x] * localPosition * weights.x;
#endif
        totalWeight += weights.x;
    }

    if (weights.y != 0.0 && indices.y < uint(boneCount))
    {
#ifdef DEKO3D
        matSkin += boneMatrices[indices.y] * weights.y;
#else
        skinnedPosition += boneMatrices[indices.y] * localPosition * weights.y;
#endif
        totalWeight += weights.y;
    }

    if (weights.z != 0.0 && indices.z < uint(boneCount))
    {
#ifdef DEKO3D
        matSkin += boneMatrices[indices.z] * weights.z;
#else
        skinnedPosition += boneMatrices[indices.z] * localPosition * weights.z;
#endif
        totalWeight += weights.z;
    }

    if (weights.w != 0.0 && indices.w < uint(boneCount))
    {
#ifdef DEKO3D
        matSkin += boneMatrices[indices.w] * weights.w;
#else
        skinnedPosition += boneMatrices[indices.w] * localPosition * weights.w;
#endif
        totalWeight += weights.w;
    }

#ifdef DEKO3D
    return totalWeight > 0.0 ? matSkin * localPosition : position;
#else
    return totalWeight > 0.0 ? skinnedPosition.xyz : position.xyz;
#endif
}
