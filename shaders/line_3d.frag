#version 450

layout(location = 0) out vec4 o_color;
layout(set = 3, binding = 0) uniform t_color
{
    vec4 u_color;
};

void main()
{
    o_color = u_color;
}