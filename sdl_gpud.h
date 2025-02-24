/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
*/

#ifndef SDL_GPUD_H
#define SDL_GPUD_H

#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 
 */
typedef struct
{
    float x;
    float y;
    float z;
}
SDL_GPUDPoint;

/**
 * @brief Initialize the debugging library.
 * @param device The device.
 * @param color_format The format of the color texture.
 * @param depth_format The format of the depth texture.
 * @return True on success or false on failure.
 */
bool SDL_InitGPUD(
    SDL_GPUDevice* device,
    const SDL_GPUTextureFormat color_format,
    const SDL_GPUTextureFormat depth_format);

/**
 * @brief Shutdown the debugging library.
 */
void SDL_QuitGPUD();

/**
 * @brief Push a color for subsequent drawing operations.
 * @param color The color.
 * @return True on success or false on failure.
 */
bool SDL_GPUDColor(
    const SDL_FColor* color);

/**
 * @brief Push a line to draw.
 * @param start The start position of the line.
 * @param end The end position of the line.
 * @return True on success or false on failure.
 */
bool SDL_GPUDLine(
    const SDL_GPUDPoint* start,
    const SDL_GPUDPoint* end);

/**
 * @brief 
 * @param position 
 * @param size 
 * @param text 
 * @return
 */
bool SDL_GPUDText(
    const SDL_FPoint* position,
    const float size,
    const char* text);

/**
 * @brief Draw all batched commands.
 * @param command_buffer The command buffer.
 * @param color_texture The color texture (usually swapchain texture).
 * @param depth_texture The depth texture (or NULL for 2D rendering).
 * @param matrix The matrix (should be float[16] or equivalent).
 * @return True on success or false on failure.
 * @note You should be clearing the textures yourself before calling.
 */
bool SDL_GPUDDraw(
    SDL_GPUCommandBuffer* command_buffer,
    SDL_GPUTexture* color_texture,
    SDL_GPUTexture* depth_texture,
    const void* matrix);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus

#include <type_traits>

/**
 * @copydoc SDL_GPUDColor
 */
inline bool SDL_GPUDColor(
    const SDL_FColor& color)
{
    return SDL_GPUDColor(&color);
}

/**
 * @copydoc SDL_GPUDLine
 */
bool SDL_GPUDLine(
    const SDL_GPUDPoint& start,
    const SDL_GPUDPoint& end)
{
    return SDL_GPUDLine(&start, &end);
}

/**
 * @copydoc SDL_GPUDText
 */
bool SDL_GPUDText(
    const SDL_FPoint& position,
    const float size,
    const char* text)
{
    return SDL_GPUDText(&position, size, text);
}

/**
 * @copydoc SDL_GPUDDraw
 */
template<typename T> requires !std::is_pointer_v<T>
bool SDL_GPUDDraw(
    SDL_GPUCommandBuffer* command_buffer,
    SDL_GPUTexture* color_texture,
    SDL_GPUTexture* depth_texture,
    const T& matrix)
{
    return SDL_GPUDDraw(command_buffer, color_texture, depth_texture, &matrix);
}

#endif
#endif

/******************************************************************************
 * IMPLEMENTATION *************************************************************
 ******************************************************************************/

#ifdef SDL_GPUD_IMPL
#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Begin stb_easy_font ********************************************************
 ******************************************************************************/

// stb_easy_font.h - v1.1 - bitmap font for 3D rendering - public domain
// Sean Barrett, Feb 2015
//
//    Easy-to-deploy,
//    reasonably compact,
//    extremely inefficient performance-wise,
//    crappy-looking,
//    ASCII-only,
//    bitmap font for use in 3D APIs.
//
// Intended for when you just want to get some text displaying
// in a 3D app as quickly as possible.
//
// Doesn't use any textures, instead builds characters out of quads.
//
// DOCUMENTATION:
//
//   int stb_easy_font_width(char *text)
//   int stb_easy_font_height(char *text)
//
//      Takes a string and returns the horizontal size and the
//      vertical size (which can vary if 'text' has newlines).
//
//   int stb_easy_font_print(float x, float y,
//                           char *text, unsigned char color[4],
//                           void *vertex_buffer, int vbuf_size)
//
//      Takes a string (which can contain '\n') and fills out a
//      vertex buffer with renderable data to draw the string.
//      Output data assumes increasing x is rightwards, increasing y
//      is downwards.
//
//      The vertex data is divided into quads, i.e. there are four
//      vertices in the vertex buffer for each quad.
//
//      The vertices are stored in an interleaved format:
//
//         x:float
//         y:float
//         z:float
//         color:uint8[4]
//
//      You can ignore z and color if you get them from elsewhere
//      This format was chosen in the hopes it would make it
//      easier for you to reuse existing vertex-buffer-drawing code.
//
//      If you pass in NULL for color, it becomes 255,255,255,255.
//
//      Returns the number of quads.
//
//      If the buffer isn't large enough, it will truncate.
//      Expect it to use an average of ~270 bytes per character.
//
//      If your API doesn't draw quads, build a reusable index
//      list that allows you to render quads as indexed triangles.
//
//   void stb_easy_font_spacing(float spacing)
//
//      Use positive values to expand the space between characters,
//      and small negative values (no smaller than -1.5) to contract
//      the space between characters.
//
//      E.g. spacing = 1 adds one "pixel" of spacing between the
//      characters. spacing = -1 is reasonable but feels a bit too
//      compact to me; -0.5 is a reasonable compromise as long as
//      you're scaling the font up.
//
// LICENSE
//
//   See end of file for license information.
//
// VERSION HISTORY
//
//   (2020-02-02)  1.1   make everything static so can compile it in more than one src file
//   (2017-01-15)  1.0   space character takes same space as numbers; fix bad spacing of 'f'
//   (2016-01-22)  0.7   width() supports multiline text; add height()
//   (2015-09-13)  0.6   #include <math.h>; updated license
//   (2015-02-01)  0.5   First release
//
// CONTRIBUTORS
//
//   github:vassvik    --  bug report
//   github:podsvirov  --  fix multiple definition errors

#if 0
// SAMPLE CODE:
//
//    Here's sample code for old OpenGL; it's a lot more complicated
//    to make work on modern APIs, and that's your problem.
//
void print_string(float x, float y, char *text, float r, float g, float b)
{
  static char buffer[99999]; // ~500 chars
  int num_quads;

  num_quads = stb_easy_font_print(x, y, text, NULL, buffer, sizeof(buffer));

  glColor3f(r,g,b);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 16, buffer);
  glDrawArrays(GL_QUADS, 0, num_quads*4);
  glDisableClientState(GL_VERTEX_ARRAY);
}
#endif

#ifndef INCLUDE_STB_EASY_FONT_H
#define INCLUDE_STB_EASY_FONT_H

#include <stdlib.h>
#include <math.h>

static struct stb_easy_font_info_struct {
    unsigned char advance;
    unsigned char h_seg;
    unsigned char v_seg;
} stb_easy_font_charinfo[96] = {
    {  6,  0,  0 },  {  3,  0,  0 },  {  5,  1,  1 },  {  7,  1,  4 },
    {  7,  3,  7 },  {  7,  6, 12 },  {  7,  8, 19 },  {  4, 16, 21 },
    {  4, 17, 22 },  {  4, 19, 23 },  { 23, 21, 24 },  { 23, 22, 31 },
    { 20, 23, 34 },  { 22, 23, 36 },  { 19, 24, 36 },  { 21, 25, 36 },
    {  6, 25, 39 },  {  6, 27, 43 },  {  6, 28, 45 },  {  6, 30, 49 },
    {  6, 33, 53 },  {  6, 34, 57 },  {  6, 40, 58 },  {  6, 46, 59 },
    {  6, 47, 62 },  {  6, 55, 64 },  { 19, 57, 68 },  { 20, 59, 68 },
    { 21, 61, 69 },  { 22, 66, 69 },  { 21, 68, 69 },  {  7, 73, 69 },
    {  9, 75, 74 },  {  6, 78, 81 },  {  6, 80, 85 },  {  6, 83, 90 },
    {  6, 85, 91 },  {  6, 87, 95 },  {  6, 90, 96 },  {  7, 92, 97 },
    {  6, 96,102 },  {  5, 97,106 },  {  6, 99,107 },  {  6,100,110 },
    {  6,100,115 },  {  7,101,116 },  {  6,101,121 },  {  6,101,125 },
    {  6,102,129 },  {  7,103,133 },  {  6,104,140 },  {  6,105,145 },
    {  7,107,149 },  {  6,108,151 },  {  7,109,155 },  {  7,109,160 },
    {  7,109,165 },  {  7,118,167 },  {  6,118,172 },  {  4,120,176 },
    {  6,122,177 },  {  4,122,181 },  { 23,124,182 },  { 22,129,182 },
    {  4,130,182 },  { 22,131,183 },  {  6,133,187 },  { 22,135,191 },
    {  6,137,192 },  { 22,139,196 },  {  6,144,197 },  { 22,147,198 },
    {  6,150,202 },  { 19,151,206 },  { 21,152,207 },  {  6,155,209 },
    {  3,160,210 },  { 23,160,211 },  { 22,164,216 },  { 22,165,220 },
    { 22,167,224 },  { 22,169,228 },  { 21,171,232 },  { 21,173,233 },
    {  5,178,233 },  { 22,179,234 },  { 23,180,238 },  { 23,180,243 },
    { 23,180,248 },  { 22,189,248 },  { 22,191,252 },  {  5,196,252 },
    {  3,203,252 },  {  5,203,253 },  { 22,210,253 },  {  0,214,253 },
};

static unsigned char stb_easy_font_hseg[214] = {
   97,37,69,84,28,51,2,18,10,49,98,41,65,25,81,105,33,9,97,1,97,37,37,36,
    81,10,98,107,3,100,3,99,58,51,4,99,58,8,73,81,10,50,98,8,73,81,4,10,50,
    98,8,25,33,65,81,10,50,17,65,97,25,33,25,49,9,65,20,68,1,65,25,49,41,
    11,105,13,101,76,10,50,10,50,98,11,99,10,98,11,50,99,11,50,11,99,8,57,
    58,3,99,99,107,10,10,11,10,99,11,5,100,41,65,57,41,65,9,17,81,97,3,107,
    9,97,1,97,33,25,9,25,41,100,41,26,82,42,98,27,83,42,98,26,51,82,8,41,
    35,8,10,26,82,114,42,1,114,8,9,73,57,81,41,97,18,8,8,25,26,26,82,26,82,
    26,82,41,25,33,82,26,49,73,35,90,17,81,41,65,57,41,65,25,81,90,114,20,
    84,73,57,41,49,25,33,65,81,9,97,1,97,25,33,65,81,57,33,25,41,25,
};

static unsigned char stb_easy_font_vseg[253] = {
   4,2,8,10,15,8,15,33,8,15,8,73,82,73,57,41,82,10,82,18,66,10,21,29,1,65,
    27,8,27,9,65,8,10,50,97,74,66,42,10,21,57,41,29,25,14,81,73,57,26,8,8,
    26,66,3,8,8,15,19,21,90,58,26,18,66,18,105,89,28,74,17,8,73,57,26,21,
    8,42,41,42,8,28,22,8,8,30,7,8,8,26,66,21,7,8,8,29,7,7,21,8,8,8,59,7,8,
    8,15,29,8,8,14,7,57,43,10,82,7,7,25,42,25,15,7,25,41,15,21,105,105,29,
    7,57,57,26,21,105,73,97,89,28,97,7,57,58,26,82,18,57,57,74,8,30,6,8,8,
    14,3,58,90,58,11,7,74,43,74,15,2,82,2,42,75,42,10,67,57,41,10,7,2,42,
    74,106,15,2,35,8,8,29,7,8,8,59,35,51,8,8,15,35,30,35,8,8,30,7,8,8,60,
    36,8,45,7,7,36,8,43,8,44,21,8,8,44,35,8,8,43,23,8,8,43,35,8,8,31,21,15,
    20,8,8,28,18,58,89,58,26,21,89,73,89,29,20,8,8,30,7,
};

typedef struct
{
   unsigned char c[4];
} stb_easy_font_color;

static int stb_easy_font_draw_segs(float x, float y, unsigned char *segs, int num_segs, int vertical, stb_easy_font_color c, char *vbuf, int vbuf_size, int offset)
{
    int i,j;
    for (i=0; i < num_segs; ++i) {
        int len = segs[i] & 7;
        x += (float) ((segs[i] >> 3) & 1);
        if (len && offset+64 <= vbuf_size) {
            float y0 = y + (float) (segs[i]>>4);
            for (j=0; j < 4; ++j) {
                * (float *) (vbuf+offset+0) = x  + (j==1 || j==2 ? (vertical ? 1 : len) : 0);
                * (float *) (vbuf+offset+4) = y0 + (    j >= 2   ? (vertical ? len : 1) : 0);
                * (float *) (vbuf+offset+8) = 0.f;
                * (stb_easy_font_color *) (vbuf+offset+12) = c;
                offset += 16;
            }
        }
    }
    return offset;
}

static float stb_easy_font_spacing_val = 0;
static void stb_easy_font_spacing(float spacing)
{
   stb_easy_font_spacing_val = spacing;
}

static int stb_easy_font_print(float x, float y, char *text, unsigned char color[4], void *vertex_buffer, int vbuf_size)
{
    char *vbuf = (char *) vertex_buffer;
    float start_x = x;
    int offset = 0;

    stb_easy_font_color c = { 255,255,255,255 }; // use structure copying to avoid needing depending on memcpy()
    if (color) { c.c[0] = color[0]; c.c[1] = color[1]; c.c[2] = color[2]; c.c[3] = color[3]; }

    while (*text && offset < vbuf_size) {
        if (*text == '\n') {
            y += 12;
            x = start_x;
        } else {
            unsigned char advance = stb_easy_font_charinfo[*text-32].advance;
            float y_ch = advance & 16 ? y+1 : y;
            int h_seg, v_seg, num_h, num_v;
            h_seg = stb_easy_font_charinfo[*text-32  ].h_seg;
            v_seg = stb_easy_font_charinfo[*text-32  ].v_seg;
            num_h = stb_easy_font_charinfo[*text-32+1].h_seg - h_seg;
            num_v = stb_easy_font_charinfo[*text-32+1].v_seg - v_seg;
            offset = stb_easy_font_draw_segs(x, y_ch, &stb_easy_font_hseg[h_seg], num_h, 0, c, vbuf, vbuf_size, offset);
            offset = stb_easy_font_draw_segs(x, y_ch, &stb_easy_font_vseg[v_seg], num_v, 1, c, vbuf, vbuf_size, offset);
            x += advance & 15;
            x += stb_easy_font_spacing_val;
        }
        ++text;
    }
    return (unsigned) offset/64;
}

static int stb_easy_font_width(char *text)
{
    float len = 0;
    float max_len = 0;
    while (*text) {
        if (*text == '\n') {
            if (len > max_len) max_len = len;
            len = 0;
        } else {
            len += stb_easy_font_charinfo[*text-32].advance & 15;
            len += stb_easy_font_spacing_val;
        }
        ++text;
    }
    if (len > max_len) max_len = len;
    return (int) ceil(max_len);
}

static int stb_easy_font_height(char *text)
{
    float y = 0;
    int nonempty_line=0;
    while (*text) {
        if (*text == '\n') {
            y += 12;
            nonempty_line = 0;
        } else {
            nonempty_line = 1;
        }
        ++text;
    }
    return (int) ceil(y + (nonempty_line ? 12 : 0));
}
#endif

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

/******************************************************************************
 * End stb_easy_font **********************************************************
 ******************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sdl_gpud_shaders.h"

#define BUFFER_CAPACITY 1024

typedef struct
{
    float x;
    float y;
    float z;
    uint32_t color;
}
point_t;

typedef struct
{
    SDL_GPUTransferBuffer* cpu;
    SDL_GPUBuffer* gpu;
    uint32_t size;
    uint32_t capacity;
    uint8_t* data;
}
batch_t;

typedef enum
{
    COMMAND_TYPE_LINE,
    COMMAND_TYPE_POLY,
    COMMAND_TYPE_FONT,
}
command_type_t;

typedef struct command command_t;
typedef struct command
{
    command_t* next;
    command_type_t type;
    batch_t vbo;
    batch_t ibo;
}
command_t;

static SDL_GPUDevice* device;
static SDL_GPUGraphicsPipeline* line2d_pipeline;
static SDL_GPUGraphicsPipeline* line3d_pipeline;
static SDL_GPUGraphicsPipeline* poly2d_pipeline;
static SDL_GPUGraphicsPipeline* poly3d_pipeline;
static SDL_GPUGraphicsPipeline* font2d_pipeline;
static command_t* command_head;
static command_t* command_tail;
static uint32_t color;

/******************************************************************************
 * INITIALIZATION *************************************************************
 ******************************************************************************/

bool SDL_InitGPUD(
    SDL_GPUDevice* handle,
    const SDL_GPUTextureFormat color_format,
    const SDL_GPUTextureFormat depth_format)
{
    device = handle;
    if (!device)
    {
        return SDL_InvalidParamError("device");
    }
    typedef enum
    {
        SHADER_POLY_VERT,
        SHADER_POLY_FRAG,
        SHADER_FONT_VERT,
        SHADER_FONT_FRAG,
        SHADER_COUNT,
    }
    shader_t;
    struct
    {
        SDL_GPUShader* shader;
        SDL_GPUShaderStage stage;
        unsigned char* spv_data;
        unsigned char* dxil_data;
        unsigned char* msl_data;
        size_t spv_size;
        size_t dxil_size;
        size_t msl_size;
        int uniforms;
    }
    shaders[SHADER_COUNT] =
    {
        [SHADER_POLY_VERT] =
        {
            .stage = SDL_GPU_SHADERSTAGE_VERTEX,
            .spv_data = shaders_bin_poly_vert_spv,
            .dxil_data = shaders_bin_poly_vert_dxil,
            .msl_data = shaders_bin_poly_vert_msl,
            .spv_size = shaders_bin_poly_vert_spv_len,
            .dxil_size = shaders_bin_poly_vert_dxil_len,
            .msl_size = shaders_bin_poly_vert_msl_len,
            .uniforms = 1,
        },
        [SHADER_POLY_FRAG] =
        {
            .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
            .spv_data = shaders_bin_poly_frag_spv,
            .dxil_data = shaders_bin_poly_frag_dxil,
            .msl_data = shaders_bin_poly_frag_msl,
            .spv_size = shaders_bin_poly_frag_spv_len,
            .dxil_size = shaders_bin_poly_frag_dxil_len,
            .msl_size = shaders_bin_poly_frag_msl_len,
            .uniforms = 0,
        },
        [SHADER_FONT_VERT] =
        {
            .stage = SDL_GPU_SHADERSTAGE_VERTEX,
            .spv_data = shaders_bin_font_vert_spv,
            .dxil_data = shaders_bin_font_vert_dxil,
            .msl_data = shaders_bin_font_vert_msl,
            .spv_size = shaders_bin_font_vert_spv_len,
            .dxil_size = shaders_bin_font_vert_dxil_len,
            .msl_size = shaders_bin_font_vert_msl_len,
            .uniforms = 0,
        },
        [SHADER_FONT_FRAG] =
        {
            .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
            .spv_data = shaders_bin_font_frag_spv,
            .dxil_data = shaders_bin_font_frag_dxil,
            .msl_data = shaders_bin_font_frag_msl,
            .spv_size = shaders_bin_font_frag_spv_len,
            .dxil_size = shaders_bin_font_frag_dxil_len,
            .msl_size = shaders_bin_font_frag_msl_len,
            .uniforms = 0,
        },
    };
    SDL_GPUShaderFormat format;
    const char* entrypoint;
    if (SDL_GetGPUShaderFormats(device) & SDL_GPU_SHADERFORMAT_SPIRV)
    {
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
    }
    else if (SDL_GetGPUShaderFormats(device) & SDL_GPU_SHADERFORMAT_DXIL)
    {
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
    }
    else if (SDL_GetGPUShaderFormats(device) & SDL_GPU_SHADERFORMAT_MSL)
    {
        format = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main0";
    }
    else
    {
        return SDL_Unsupported();
    }
    for (shader_t shader = 0; shader < SHADER_COUNT; shader++)
    {
        SDL_GPUShaderCreateInfo info = {0};
        switch (format)
        {
        case SDL_GPU_SHADERFORMAT_SPIRV:
            info.code = shaders[shader].spv_data;
            info.code_size = shaders[shader].spv_size;
            break;
        case SDL_GPU_SHADERFORMAT_DXIL:
            info.code = shaders[shader].dxil_data;
            info.code_size = shaders[shader].dxil_size;
            break;
        case SDL_GPU_SHADERFORMAT_MSL:
            info.code = shaders[shader].msl_data;
            info.code_size = shaders[shader].msl_size;
            break;
        }
        info.format = format;
        info.stage = shaders[shader].stage;
        info.entrypoint = entrypoint;
        info.num_uniform_buffers = shaders[shader].uniforms;
        shaders[shader].shader = SDL_CreateGPUShader(device, &info);
        if (!shaders[shader].shader)
        {
            goto error;
        }
    }
    SDL_GPUGraphicsPipelineCreateInfo info =
    {
        .target_info =
        {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[])
            {{
                .format = color_format,
                .blend_state =
                {
                    .enable_blend = true,
                    .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                    .color_blend_op = SDL_GPU_BLENDOP_ADD,
                    .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                    .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                    .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                    .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                },
            }},
            .depth_stencil_format = depth_format,
        },
        .vertex_input_state =
        {
            .num_vertex_attributes = 2,
            .vertex_attributes = (SDL_GPUVertexAttribute[])
            {{
                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                .location = 0,
                .offset = 0,
            },
            {
                .format = SDL_GPU_VERTEXELEMENTFORMAT_UINT,
                .location = 1,
                .offset = 12,
            }},
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[])
            {{
                .pitch = 16,
            }},
        },
        .depth_stencil_state =
        {
            .compare_op = SDL_GPU_COMPAREOP_LESS,
            .enable_depth_test = true,
            .enable_depth_write = true,
        }
    };

    info.vertex_shader = shaders[SHADER_POLY_VERT].shader;
    info.fragment_shader = shaders[SHADER_POLY_FRAG].shader;
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
    info.target_info.has_depth_stencil_target = false;
    line2d_pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);

    info.vertex_shader = shaders[SHADER_POLY_VERT].shader,
    info.fragment_shader = shaders[SHADER_POLY_FRAG].shader,
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
    info.target_info.has_depth_stencil_target = true;
    line3d_pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);

    info.vertex_shader = shaders[SHADER_POLY_VERT].shader;
    info.fragment_shader = shaders[SHADER_POLY_FRAG].shader;
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.target_info.has_depth_stencil_target = false;
    poly2d_pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);

    info.vertex_shader = shaders[SHADER_POLY_VERT].shader,
    info.fragment_shader = shaders[SHADER_POLY_FRAG].shader,
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.target_info.has_depth_stencil_target = true;
    poly3d_pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);

    if (!line2d_pipeline ||
        !line3d_pipeline ||
        !poly2d_pipeline ||
        !poly3d_pipeline)
    {
        goto error;
    }
    bool status = true;
    goto success;
error:
    status = false;
success:
    for (shader_t shader = 0; shader < SHADER_COUNT; shader++)
    {
        SDL_ReleaseGPUShader(device, shaders[shader].shader);
    }
    if (!status)
    {
        SDL_QuitGPUD();
    }
    return status;
}

/******************************************************************************
 * SHUTDOWN *******************************************************************
 ******************************************************************************/

void SDL_QuitGPUD()
{
    if (!device)
    {
        return;
    }
    SDL_ReleaseGPUGraphicsPipeline(device, line2d_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, line3d_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, poly2d_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, poly3d_pipeline);
    line2d_pipeline = NULL;
    line3d_pipeline = NULL;
    poly2d_pipeline = NULL;
    poly3d_pipeline = NULL;
    device = NULL;
}

/******************************************************************************
 * BATCHING *******************************************************************
 ******************************************************************************/

static bool batch_push(
    batch_t* batch,
    const void* data,
    const uint32_t size,
    const SDL_GPUBufferUsageFlags usage)
{
    if (usage)
    {
        batch->capacity = SDL_max(BUFFER_CAPACITY, size);
        batch->size = 0;
        batch->gpu = SDL_CreateGPUBuffer(device, &(SDL_GPUBufferCreateInfo)
        {
            .usage = usage,
            .size = batch->capacity,
        });
        batch->cpu = SDL_CreateGPUTransferBuffer(device, &(SDL_GPUTransferBufferCreateInfo)
        {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = batch->capacity,
        });
        batch->data = SDL_MapGPUTransferBuffer(device, batch->cpu, false);
        if (!batch->gpu || !batch->cpu || !batch->data)
        {
            SDL_ReleaseGPUTransferBuffer(device, batch->cpu);
            SDL_ReleaseGPUBuffer(device, batch->gpu);
            return false;
        }
    }
    memcpy(batch->data + batch->size, data, size);
    batch->size += size;
    return true;
}

static void batch_upload(
    SDL_GPUCopyPass* copy_pass,
    batch_t* batch)
{
    SDL_UnmapGPUTransferBuffer(device, batch->cpu);
    batch->data = NULL;
    SDL_GPUTransferBufferLocation location = {0};
    SDL_GPUBufferRegion region = {0};
    location.transfer_buffer = batch->cpu;
    region.buffer = batch->gpu;
    region.size = batch->size;
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);
}

static bool command_push(
    const command_type_t type,
    const void* vbo_data,
    const uint32_t vbo_size,
    const void* ibo_data,
    const uint32_t ibo_size)
{
    SDL_GPUBufferUsageFlags usage;
    command_t* command;
    switch (type)
    {
    case COMMAND_TYPE_LINE:
    case COMMAND_TYPE_POLY:
        command = command_tail;
        if (!command || command->type != type || command->vbo.size + vbo_size > command->vbo.capacity)
        {
            command = calloc(1, sizeof(command_t));
            if (!command)
            {
                return SDL_OutOfMemory();
            }
            usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        }
        else
        {
            usage = 0;
        }
        if (!batch_push(&command->vbo, vbo_data, vbo_size, usage))
        {
            /* TODO: Can only actually free if it's not the tail element */
            free(command);
            return false;
        }
        break;
    }
    if (command == command_tail)
    {
        return true;
    }
    command->type = type;
    if (command_tail)
    {
        command_tail->next = command;
    }
    command_tail = command;
    if (!command_head)
    {
        command_head = command;
    }
    return true;
}

/******************************************************************************
 * COMMANDS *******************************************************************
 ******************************************************************************/

bool SDL_GPUDColor(
    const SDL_FColor* color_)
{
    if (!device)
    {
        return false;
    }
    if (!color_)
    {
        return SDL_InvalidParamError("color");
    }
    color = 0;
    color |= ((uint8_t) (color_->r * 255.0f) & 0xFF) << 24;
    color |= ((uint8_t) (color_->g * 255.0f) & 0xFF) << 16;
    color |= ((uint8_t) (color_->b * 255.0f) & 0xFF) << 8;
    color |= ((uint8_t) (color_->a * 255.0f) & 0xFF) << 0;
    return true;
}

bool SDL_GPUDLine(
    const SDL_GPUDPoint* start,
    const SDL_GPUDPoint* end)
{
    if (!device)
    {
        return false;
    }
    if (!start)
    {
        return SDL_InvalidParamError("start");
    }
    if (!end)
    {
        return SDL_InvalidParamError("end");
    }
    point_t points[2];
    points[0].x = start->x;
    points[0].y = start->y;
    points[0].z = start->z;
    points[0].color = color;
    points[1].x = end->x;
    points[1].y = end->y;
    points[1].z = end->z;
    points[1].color = color;
    return command_push(COMMAND_TYPE_LINE, points, sizeof(points), NULL, 0);
}

bool SDL_GPUDText(
    const SDL_FPoint* position,
    const float size,
    const char* text)
{
    return true;
}

/******************************************************************************
 * DRAWING ********************************************************************
 ******************************************************************************/

bool SDL_GPUDDraw(
    SDL_GPUCommandBuffer* command_buffer,
    SDL_GPUTexture* color_texture,
    SDL_GPUTexture* depth_texture,
    const void* matrix)
{
    if (!device)
    {
        return false;
    }
    if (!command_buffer)
    {
        return SDL_InvalidParamError("command_buffer");
    }
    if (!color_texture)
    {
        return SDL_InvalidParamError("color_texture");
    }
    if (!matrix)
    {
        return SDL_InvalidParamError("matrix");
    }
    SDL_PushGPUDebugGroup(command_buffer, "dbg_upload");
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    if (!copy_pass)
    {
        return false;
    }
    for (command_t* command = command_head; command; command = command->next)
    {
        switch (command->type)
        {
        case COMMAND_TYPE_LINE:
        case COMMAND_TYPE_POLY:
            batch_upload(copy_pass, &command->vbo);
            break;
        }
    }
    SDL_EndGPUCopyPass(copy_pass);
    SDL_PopGPUDebugGroup(command_buffer);
    SDL_PushGPUDebugGroup(command_buffer, "dbg_render");
    SDL_GPUColorTargetInfo color_info = {0};
    color_info.texture = color_texture;
    color_info.load_op = SDL_GPU_LOADOP_LOAD;
    color_info.store_op = SDL_GPU_STOREOP_STORE;
    SDL_GPURenderPass* render_pass;
    if (depth_texture)
    {
        SDL_GPUDepthStencilTargetInfo depth_info = {0};
        depth_info.texture = depth_texture;
        depth_info.load_op = SDL_GPU_LOADOP_LOAD;
        depth_info.store_op = SDL_GPU_STOREOP_STORE;
        render_pass = SDL_BeginGPURenderPass(command_buffer, &color_info, 1, &depth_info);
    }
    else
    {
        render_pass = SDL_BeginGPURenderPass(command_buffer, &color_info, 1, NULL);
    }
    if (!render_pass)
    {
        return false;
    }
    SDL_GPUGraphicsPipeline* pipeline1 = NULL;
    SDL_GPUGraphicsPipeline* pipeline2 = NULL;
    for (command_t* command = command_head; command;)
    {
        switch (command->type)
        {
        case COMMAND_TYPE_LINE:
            if (depth_texture)
            {
                pipeline2 = line3d_pipeline;
            }
            else
            {
                pipeline2 = line2d_pipeline;
            }
            break;
        case COMMAND_TYPE_POLY:
            if (depth_texture)
            {
                pipeline2 = poly3d_pipeline;
            }
            else
            {
                pipeline2 = poly2d_pipeline;
            }
            break;
        }
        if (pipeline1 != pipeline2)
        {
            pipeline1 = pipeline2;
            SDL_BindGPUGraphicsPipeline(render_pass, pipeline1);
            SDL_PushGPUVertexUniformData(command_buffer, 0, matrix, 16 * sizeof(float));
        }
        switch (command->type)
        {
        case COMMAND_TYPE_LINE:
        case COMMAND_TYPE_POLY:
            SDL_GPUBufferBinding binding = {0};
            binding.buffer = command->vbo.gpu;
            SDL_BindGPUVertexBuffers(render_pass, 0, &binding, 1);
            SDL_DrawGPUPrimitives(render_pass, command->vbo.size / (4 * sizeof(float)), 1, 0, 0);
            SDL_ReleaseGPUTransferBuffer(device, command->vbo.cpu);
            SDL_ReleaseGPUBuffer(device, command->vbo.gpu);
            break;
        }
        command_t* command_prev = command;
        command = command->next;
        free(command_prev);
    }
    SDL_EndGPURenderPass(render_pass);
    SDL_PopGPUDebugGroup(command_buffer);
    command_head = NULL;
    command_tail = NULL;
    return true;
}

#ifdef __cplusplus
}
#endif
#endif