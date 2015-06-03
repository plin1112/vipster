#version 330

uniform mat4 vpMatrix;
uniform mat3 cellVec;
uniform vec3 volOff;

layout(location=0) in vec3 vertex_cellspace;
layout(location=1) in vec3 offset;

void main(void)
{
    gl_Position = vpMatrix * vec4(vertex_cellspace*cellVec+offset+volOff,1);
}