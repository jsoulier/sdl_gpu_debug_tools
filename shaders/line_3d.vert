#version 450

layout(location = 0) in vec3 i_position;
layout(set = 1, binding = 0) uniform t_matrix
{
    mat4 u_matrix;
};

void main()
{
    gl_Position = u_matrix * vec4(i_position, 1.0f);
}
