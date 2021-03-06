layout(location = 0) in vec3 vertex_modelspace;
layout(location = 1) in mat3 mMatrix;
layout(location = 4) in vec3 position_modelspace;
layout(location = 5) in uvec4 pbc_crit;
layout(location = 6) in vec4 s1Color;
layout(location = 7) in vec4 s2Color;

layout(std140, row_major) uniform viewMat{
    mat4 vpMatrix;
    mat4 rMatrix;
};
uniform mat3 position_scale;
uniform vec3 offset;
uniform uvec3 pbc_cell;
uniform uvec3 mult;

out vec3 normals_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;
out float vertex_side;
out vec4 s1Cpass;
out vec4 s2Cpass;
flat out uint render;

void main(void)
{
    if(pbc_crit.w != uint(0)){
        render = uint(1);
    }else{
        uvec3 test = mult - pbc_crit.xyz;
        render = uint((test.x > pbc_cell.x) && (test.y > pbc_cell.y) && (test.z > pbc_cell.z));
    }
    //standard vertex positioning:
    gl_Position = vpMatrix * vec4(mMatrix * vertex_modelspace + position_modelspace * position_scale + offset, 1);
    //pass coordinate and colors to fragment shader
    vertex_side = vertex_modelspace.x;
    s1Cpass = s1Color;
    s2Cpass = s2Color;

    //transformations for calculation of lighting:
    vec3 vertex_cameraspace=(rMatrix*vec4(mMatrix*vertex_modelspace,0)).xyz;

    //from vertex to camera (in cameraspace always at origin)
    EyeDirection_cameraspace = vec3(0,0,25) - vertex_cameraspace;

    //from vertex to light source
    LightDirection_cameraspace = vec3(10,10,10) + EyeDirection_cameraspace;

    //Normals in camera space
    normals_cameraspace = (rMatrix * vec4(mMatrix * vec3(0,vertex_modelspace.yz),0)).xyz;
}
