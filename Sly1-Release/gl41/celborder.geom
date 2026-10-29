#version 410 core

// Mac-only (GL 4.1): no shared counterpart. GL 4.1 core forward-compatible
// contexts only accept glLineWidth(1), so this stage draws each celborder.vert
// edge as a quad shaped like the wide GL_LINES line the shared build draws
// with glLineWidth(celLineWidthPx). Without multisampling (the game's default)
// GL rounds a wide line's width to whole pixels and spreads it along the
// minor axis: each column of an x-major line (each row of a y-major one) gets
// that many pixels, and the ends are square to the major axis. A diagonal is
// therefore thinner across than an axis-aligned edge. With multisampling GL
// draws a rectangle perpendicular to the line at the unrounded width instead,
// ending at the endpoints; this stage draws that while celMultisampled is set.

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

// Outline width in pixels, what render.cpp used to pass to glLineWidth.
uniform float celLineWidthPx;
// Size in pixels of the viewport the outlines are drawn into.
uniform vec2 celViewportPx;
// Whether that target is multisampled (render.cpp).
uniform bool celMultisampled;

// celborder.vert's CullVertex() position: both ends of a culled edge get it.
const vec4 kCulled = vec4(2.0, 2.0, 2.0, 1.0);

void main()
{
    vec4 a = gl_in[0].gl_Position;
    vec4 b = gl_in[1].gl_Position;

    if (a.w <= 0.0 || b.w <= 0.0 || a == kCulled || b == kCulled)
        return;

    vec2 halfViewport = 0.5 * celViewportPx;
    vec2 direction = (b.xy / b.w - a.xy / a.w) * halfViewport;
    float lengthPx = length(direction);
    if (!(lengthPx > 0.0))
        return;

    // Rounded to whole pixels, at least one (floor(x + 0.5): GLSL's round()
    // may go either way at .5).
    float widthPx = max(floor(celLineWidthPx + 0.5), 1.0);

    // The minor axis, pointing to the left of the edge so the strip below is
    // counter-clockwise. An edge at exactly 45 degrees counts as x-major, as
    // in GL.
    vec2 side = abs(direction.x) >= abs(direction.y)
        ? vec2(0.0, sign(direction.x))
        : vec2(-sign(direction.y), 0.0);

    // Multisampled: the unit normal on the same side, at the unrounded width.
    if (celMultisampled)
    {
        side = vec2(-direction.y, direction.x) / lengthPx;
        widthPx = celLineWidthPx;
    }

    // Half the width, in NDC. Scaling it by each end's w keeps the band the
    // same width on screen along the whole edge once divided by w again.
    vec2 offset = side * (0.5 * widthPx) / halfViewport;

    // Counter-clockwise on screen for any direction, so GL_CULL_FACE with
    // back faces culled keeps it.
    gl_Position = vec4(a.xy - offset * a.w, a.zw);
    EmitVertex();
    gl_Position = vec4(b.xy - offset * b.w, b.zw);
    EmitVertex();
    gl_Position = vec4(a.xy + offset * a.w, a.zw);
    EmitVertex();
    gl_Position = vec4(b.xy + offset * b.w, b.zw);
    EmitVertex();
    EndPrimitive();
}
