layout(location = 0) in vec3 vertex_modelspace;
layout(std140, row_major) uniform viewMat{
    mat4 vpMatrix;
    mat4 rMatrix;
};
uniform vec3 offset;

void main(void)
{
    gl_Position = vpMatrix * vec4(vertex_modelspace+offset,1);
}

