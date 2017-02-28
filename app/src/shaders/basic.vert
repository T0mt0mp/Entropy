#version 330 core

/// Vertex position in model space.
layout(location = 0) in vec3 vertexPosMS;

/// Model-View-Projection matrix.
uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(vertexPosMS, 1.0);
}

