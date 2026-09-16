#version 430 core

layout(location = 0) in vec2 corner;
layout(location = 1) in vec4 instancePosCreated;
layout(location = 2) in vec4 instanceVelocity;
layout(location = 3) in vec4 instanceColor;
layout(location = 4) in vec4 instanceRoll;

layout(std140, binding = 0) uniform CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

uniform mat3 cameraMat;
uniform float currentTime;
uniform float dtFrame;

layout(std140, binding = 1) uniform BlipGroup
{
    mat4 model;
    vec4 params;
    vec4 clqScaleData;
    vec4 clqAlphaData;
} group;

out vec2 fragUv;
out vec4 fragColor;

float evaluateClq(vec3 clq, float t)
{
    return clq.x + t * (clq.y + t * clq.z);
}

void main()
{
    // Camera matrix columns match the engine's CM basis. Normalize in the
    // shader so the entire basis is supplied with one uniform upload.
    vec3 cameraForward = normalize(cameraMat[0]);
    vec3 cameraRight = normalize(cameraMat[1]);
    vec3 cameraUp = normalize(cameraMat[2]);

    // Match the VU1 programs: creation times and CLQ scale are not clamped.
    // A negative scale is meaningful and mirrors the billboard through zero.
    float age = currentTime - instancePosCreated.w;
    float scale = evaluateClq(group.clqScaleData.xyz, age);
    float alpha = clamp(evaluateClq(group.clqAlphaData.xyz, age), 0.0, 1.0) * group.params.x;
    int blipMode = int(group.params.z);
    vec3 center = (group.model * vec4(instancePosCreated.xyz, 1.0)).xyz;
    vec3 velocity = mat3(group.model) * instanceVelocity.xyz;
    vec3 worldPos;
    vec4 clipPos;

    if (blipMode == 1)
    {
        // The PS2 flying program builds a velocity-aligned strip and ensures
        // that its projected center line is at least one sprite diameter.
        // Flying BLIPs use the distance traveled during this frame, not the
        // raw per-second velocity.  The original VU program receives dtFrame
        // in RBEX for this conversion.
        // The retail Flying VU program constructs the strip after projection.
        // Work in NDC so its minimum length and perpendicular are measured in
        // screen space, while retaining velocity along the camera-forward axis.
        vec3 lineWorld = velocity * (group.params.y * dtFrame);
        vec4 centerClip = cm.matWorldToClip * vec4(center, 1.0);
        vec4 headClip = cm.matWorldToClip * vec4(center + lineWorld, 1.0);
        float centerW = abs(centerClip.w) > 0.0001 ? centerClip.w : 0.0001;
        float headW = abs(headClip.w) > 0.0001 ? headClip.w : 0.0001;
        vec2 centerNdc = centerClip.xy / centerW;
        vec2 headNdc = headClip.xy / headW;
        vec2 lineNdc = headNdc - centerNdc;

        vec4 rightClip = cm.matWorldToClip * vec4(center + cameraRight * scale, 1.0);
        vec4 upClip = cm.matWorldToClip * vec4(center + cameraUp * scale, 1.0);
        float rightW = abs(rightClip.w) > 0.0001 ? rightClip.w : 0.0001;
        float upW = abs(upClip.w) > 0.0001 ? upClip.w : 0.0001;
        float halfWidth = max(length(rightClip.xy / rightW - centerNdc), length(upClip.xy / upW - centerNdc));
        float lineLength = length(lineNdc);
        float diameter = halfWidth * 2.0;
        if (lineLength < diameter)
        {
            vec2 fallback = lineLength > 0.0001 ? lineNdc / lineLength : vec2(0.0, 1.0);
            lineNdc = fallback * diameter;
        }

        vec2 across = vec2(-lineNdc.y, lineNdc.x);
        float acrossLength = length(across);
        across = acrossLength > 0.0001 ? across * (halfWidth / acrossLength)
                                      : vec2(halfWidth, 0.0);
        float along = corner.y * 0.5 + 0.5;
        vec4 depthClip = mix(centerClip, headClip, along);
        vec2 vertexNdc = centerNdc + lineNdc * along + across * corner.x;
        clipPos = vec4(vertexNdc * depthClip.w, depthClip.z, depthClip.w);
    }
    else
    {
        vec2 billboardCorner = corner;
        if (blipMode == 2)
        {
            float s = sin(instanceRoll.x);
            float c = cos(instanceRoll.x);
            billboardCorner = mat2(c, s, -s, c) * billboardCorner;
        }
        worldPos = center + (cameraRight * billboardCorner.x + cameraUp * billboardCorner.y) * scale;
    }

    fragUv = vec2(corner.x * 0.5 + 0.5, 1.0 - (corner.y * 0.5 + 0.5));
    fragColor = vec4(instanceColor.rgb, instanceColor.a * alpha);
    gl_Position = blipMode == 1 ? clipPos : cm.matWorldToClip * vec4(worldPos, 1.0);
}
