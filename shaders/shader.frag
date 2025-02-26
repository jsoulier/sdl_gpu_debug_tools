#version 450

layout(location = 0) in flat uint i_color;
layout(location = 0) out vec4 o_color;

void main()
{
    const uint red = i_color >> 24 & 0xFF;
    const uint green = i_color >> 16 & 0xFF;
    const uint blue = i_color >> 8 & 0xFF;
    const uint alpha = i_color >> 0 & 0xFF;
    o_color = vec4(red, green, blue, alpha) / 255.0f;
}