#version 330 core

/// Vertex position in model space.
layout(location = 0) in vec3 vertexPosMS;
/// Vertex color.
layout(location = 1) in vec3 vertColor;
/// Model matrix. Actualy takes up locations 2, 3, 4 and 5.
layout(location = 2) in mat4 model;

out vec3 fragColor;

/// View-Projection matrix.
uniform mat4 vp;

void main()
{
    gl_Position = vp * model * vec4(vertexPosMS, 1.0);
    fragColor = vertColor;
}

