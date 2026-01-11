#version 430 core

layout (std430, binding = 0) readonly buffer CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

layout (std140, binding = 1) uniform ROCEL // Cel border properties
{
    mat4  model;                 
	vec4  celRgba;         
	float uAlphaCelBorder;
}cbp;

layout (std430, binding = 5) readonly buffer EdgeBuffer 
{
    vec4 edges[];
};

void main()
{
    // Which edge are we on? (2 vertices per edge)
    int  edgeID = gl_VertexID >> 1;
    bool first  = (gl_VertexID & 1) == 0;

    // Fetch edge block (object space)
    vec3 E0 = edges[edgeID * 4 + 0].xyz;
    vec3 E1 = edges[edgeID * 4 + 1].xyz;
    vec3 OA = edges[edgeID * 4 + 2].xyz;
    vec3 OB = edges[edgeID * 4 + 3].xyz;

    // Object -> World
    vec3 E0w = (cbp.model * vec4(E0, 1.0)).xyz;
    vec3 E1w = (cbp.model * vec4(E1, 1.0)).xyz;
    vec3 OAw = (cbp.model * vec4(OA, 1.0)).xyz;
    vec3 OBw = (cbp.model * vec4(OB, 1.0)).xyz;

    // World -> Clip
    vec4 A = cm.matWorldToClip * vec4(E0w, 1.0);
    vec4 B = cm.matWorldToClip * vec4(E1w, 1.0);
    vec4 C = cm.matWorldToClip * vec4(OAw, 1.0);
    vec4 D = cm.matWorldToClip * vec4(OBw, 1.0);

    // Validity: only endpoints must be in front of camera
    bool edgeValid = (A.w > 0.0 && B.w > 0.0);
    bool oppValid  = (C.w > 0.0 && D.w > 0.0);

    // --- World-space silhouette test (stable near frustum edges) ---
    // Edge direction
    vec3 e = E1w - E0w;

    // Build the two adjacent face normals around the shared edge.
    // Winding/order matters; these two forms generally give opposing normals for adjacent tris.
    vec3 n0 = cross(e, OAw - E0w);
    vec3 n1 = cross(OBw - E0w, e);

    // If either normal is degenerate, treat as border-ish.
    float n0Len2 = dot(n0, n0);
    float n1Len2 = dot(n1, n1);

    // View direction from edge midpoint to camera (world space)
    vec3 mid = 0.5 * (E0w + E1w);
    vec3 v   = cm.cameraPos.xyz - mid;

    // Opposite-facing if one tri faces camera and the other faces away
    bool opposite = false;
    if (n0Len2 > 1e-20 && n1Len2 > 1e-20 && dot(v, v) > 1e-20)
    {
        vec3 n0N = n0 * inversesqrt(n0Len2);
        vec3 n1N = n1 * inversesqrt(n1Len2);
        vec3 vN  = v  * inversesqrt(dot(v, v));

        float f0 = dot(n0N, vN);
        float f1 = dot(n1N, vN);

        // small epsilon to avoid flicker when nearly perpendicular
        const float kFaceEPS = 1e-4;
        opposite = (f0 >  kFaceEPS && f1 < -kFaceEPS) || (f1 >  kFaceEPS && f0 < -kFaceEPS);
    }

    // Border if opposite info isn't usable (behind camera / degenerate)
    bool isBorder = (!oppValid) || (n0Len2 <= 1e-20) || (n1Len2 <= 1e-20);

    // Decide whether we want this edge (same intent as your original comment)
    bool drawEdge = edgeValid && (isBorder || opposite);

    if (!drawEdge)
    {
        // Push it outside clip so the fragment shader doesn't run
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }

    // Emit one endpoint of the edge
    vec4 P = first ? A : B;

    // --- Depth: per-vertex depth that still respects opposite triangle ---
    float dP = (P.z / P.w) * 0.5 + 0.5;

    // If opposite is not valid, don't clamp (treat like border)
    float zLine01 = dP;
    if (oppValid)
    {
        float dC = (C.z / C.w) * 0.5 + 0.5;
        float dD = (D.z / D.w) * 0.5 + 0.5;
        float zOppNear = min(dC, dD);
        zLine01 = max(dP, zOppNear);
    }

    const float kBias01 = 1e-5; // tune: 1e-5..2e-4
    float zBiased = clamp(zLine01 - kBias01, 0.0, 1.0);

    float zNDC = zBiased * 2.0 - 1.0;
    P.z = zNDC * P.w;

    gl_Position = P;
}
