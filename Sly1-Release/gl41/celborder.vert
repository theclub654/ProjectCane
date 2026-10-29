#version 430 core


layout(std140) uniform CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

layout(std140) uniform ROCEL
{
    mat4 model;
    vec4 celRgba;
    float uAlphaCelBorder;
} cbp;

// Buffer textures, since GLSL 4.10 has no buffer blocks. Texture units are set
// from C++ (kSamplerUnits in glshaders.h).
// Edge points: GLOB::edgeSSBO viewed as RGBA32UI, three texels per CELPOINT
// (vec4 position, uvec4 boneIndices, vec4 boneWeights), four CELPOINTs per
// edge (E0, E1, OA, OB). The float parts come back through uintBitsToFloat.
uniform usamplerBuffer edgeTex;
// Bone matrices: GLOBSET::boneMatrixSSBO, a column-major mat4 per bone, one
// column per RGBA32F texel.
uniform samplerBuffer boneMatrixTex;
// Pose deltas: GLOB::edgePoseDposSSBO, poseCount RGBA32F texels per CELPOINT.
uniform samplerBuffer edgePoseDposTex;
// Pose weights: the globset's agPoses, one R32F texel per pose.
uniform samplerBuffer poseWeightTex;

struct CELPOINT
{
    vec4 position;
    uvec4 boneIndices;
    vec4 boneWeights;
};

int EdgePointCount()
{
    return textureSize(edgeTex) / 3;
}

CELPOINT EdgePoint(int pointIndex)
{
    int texel = pointIndex * 3;
    CELPOINT point;
    point.position = uintBitsToFloat(texelFetch(edgeTex, texel));
    point.boneIndices = texelFetch(edgeTex, texel + 1);
    point.boneWeights = uintBitsToFloat(texelFetch(edgeTex, texel + 2));
    return point;
}

mat4 BoneMatrix(uint bone)
{
    int texel = int(bone) * 4;
    return mat4(texelFetch(boneMatrixTex, texel),
                texelFetch(boneMatrixTex, texel + 1),
                texelFetch(boneMatrixTex, texel + 2),
                texelFetch(boneMatrixTex, texel + 3));
}

uniform int fSkin;
uniform int fPose;
uniform int poseCount;

void CullVertex();
float Cross2D(vec2 a, vec2 b);
vec3 GetEdgePosition(int pointIndex);

void main()
{
    int edgeID = gl_VertexID >> 1;
    bool first = (gl_VertexID & 1) == 0;
    int pointBase = edgeID * 4;

    if (pointBase < 0 || pointBase + 3 >= EdgePointCount())
    {
        CullVertex();
        return;
    }

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
    CELPOINT point = EdgePoint(pointIndex);
    vec4 position = point.position;

    if (fPose != 0)
    {
        int count = min(max(poseCount, 0), textureSize(poseWeightTex));
        int poseBase = pointIndex * poseCount;

        for (int ipose = 0; ipose < count; ++ipose)
        {
            float weight = texelFetch(poseWeightTex, ipose).r;

            int poseIndex = poseBase + ipose;
            if (weight != 0.0 && poseIndex >= 0 &&
                poseIndex < textureSize(edgePoseDposTex))
                position.xyz += texelFetch(edgePoseDposTex, poseIndex).xyz * weight;
        }
    }

    if (fSkin == 0)
        return position.xyz;

    if (pointIndex < 0 || pointIndex >= EdgePointCount())
        return position.xyz;

    uvec4 indices = point.boneIndices;
    vec4 weights = point.boneWeights;
    vec4 localPosition = vec4(position.xyz, 1.0);
    vec4 skinnedPosition = vec4(0.0);
    float totalWeight = 0.0;
    int boneCount = textureSize(boneMatrixTex) / 4;

    if (weights.x != 0.0 && indices.x < uint(boneCount))
    {
        skinnedPosition += BoneMatrix(indices.x) * localPosition * weights.x;
        totalWeight += weights.x;
    }

    if (weights.y != 0.0 && indices.y < uint(boneCount))
    {
        skinnedPosition += BoneMatrix(indices.y) * localPosition * weights.y;
        totalWeight += weights.y;
    }

    if (weights.z != 0.0 && indices.z < uint(boneCount))
    {
        skinnedPosition += BoneMatrix(indices.z) * localPosition * weights.z;
        totalWeight += weights.z;
    }

    if (weights.w != 0.0 && indices.w < uint(boneCount))
    {
        skinnedPosition += BoneMatrix(indices.w) * localPosition * weights.w;
        totalWeight += weights.w;
    }

    return totalWeight > 0.0 ? skinnedPosition.xyz : position.xyz;
}
