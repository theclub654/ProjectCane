#version 330 core

layout (lines_adjacency) in;
layout (line_strip, max_vertices = 3) out;

void main()
{
    vec3 vertex0 = gl_in[0].gl_Position.xyz;
    vec3 vertex1 = gl_in[1].gl_Position.xyz;
    vec3 vertex2 = gl_in[2].gl_Position.xyz;
    vec3 vertex3 = gl_in[3].gl_Position.xyz;

    vec3 edge0 = vertex1 - vertex2;
    vec3 edge1 = vertex0 - vertex3;

	for (int i = 0; i < 2; i++)
    {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}