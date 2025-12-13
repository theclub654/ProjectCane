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

layout (std430, binding = 4) readonly buffer EdgeBuffer 
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

    // World ? Clip
    vec4 A = cm.matWorldToClip * (cbp.model * vec4(E0, 1.0));
    vec4 B = cm.matWorldToClip * (cbp.model * vec4(E1, 1.0));
    vec4 C = cm.matWorldToClip * (cbp.model * vec4(OA, 1.0));
    vec4 D = cm.matWorldToClip * (cbp.model * vec4(OB, 1.0));

    // Keep the original validity rule
    bool valid = (A.w > 0.0 && B.w > 0.0 && C.w > 0.0 && D.w > 0.0);

    // --- Homogeneous (pre-divide) orientation tests ---
    float sAC = A.x*(B.y*C.w - B.w*C.y)
              - A.y*(B.x*C.w - B.w*C.x)
              + A.w*(B.x*C.y - B.y*C.x);

    float sBD = A.x*(B.y*D.w - B.w*D.y)
              - A.y*(B.x*D.w - B.w*D.x)
              + A.w*(B.x*D.y - B.y*D.x);

    // Border or opposite-facing neighbor means "draw"
    const float kEPS = 1e-6;
    bool isBorder = (abs(sAC) <= kEPS) || (abs(sBD) <= kEPS);
    bool opposite = (sAC * sBD < 0.0);
    
    int fNonCelBorder = int(valid && (isBorder || opposite));

    if (fNonCelBorder == 1)
    {
        // Pushing celborder to clip space so hardware clips it and fragment shader doesnt run
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }

    // Emit one endpoint of the line for this VS invocation
    vec4 P = first ? A : B;

    // Depth for the line (no reversed-Z, no uniform bias)
    float dA = (A.z / A.w) * 0.5 + 0.5;
    float dB = (B.z / B.w) * 0.5 + 0.5;
    float dC = (C.z / C.w) * 0.5 + 0.5;
    float dD = (D.z / D.w) * 0.5 + 0.5;

    // Place the line at the *farthest* edge endpoint,
    // but never closer than the nearer opposite vertex
    float zEdgeMax = max(dA, dB);
    float zOppNear = min(dC, dD);
    float zLine01  = max(zEdgeMax, zOppNear);

    // Tiny fixed forward bias so it wins against its own surface,
    // but won't jump in front of unrelated, nearer geometry.
    const float kBias01 = 1e-5;   // tune: try 1e-5..2e-4
    float zBiased = clamp(zLine01 - kBias01, 0.0, 1.0);

    // back to clip-space
    float zNDC = zBiased * 2.0 - 1.0;
    P.z        = zNDC * P.w;

    gl_Position = P;
}