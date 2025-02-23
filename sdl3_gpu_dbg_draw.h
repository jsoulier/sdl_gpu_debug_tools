/*
 * This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <https://unlicense.org>
 */

#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>

/**
 * @brief 
 * @param device 
 * @param color_texture_format
 * @param depth_texture_format
 * @return
 */
bool SDL_InitGPUDbgDraw(
    SDL_GPUDevice* device,
    const SDL_GPUTextureFormat color_texture_format,
    const SDL_GPUTextureFormat depth_texture_format);

/**
 * @brief 
 */
void SDL_QuitGPUDbgDraw();

/**
 * @brief 
 * @param command_buffer 
 * @param depth_texture 
 * @param color_texture 
 * @param matrix 
 */
bool SDL_BeginGPUDbgPass3D(
    SDL_GPUCommandBuffer* command_buffer,
    SDL_GPUTexture* depth_texture,
    SDL_GPUTexture* color_texture,
    float* matrix);

/**
 * @brief 
 */
void SDL_EndGPUDbgPass();

/**
 * @brief 
 * @param red 
 * @param green 
 * @param blue 
 * @param alpha 
 */
void SDL_SetGPUDbgColor(
    const float red,
    const float green,
    const float blue,
    const float alpha);

/**
 * @brief 
 * @param x1 
 * @param y1 
 * @param z1 
 * @param x2 
 * @param y2 
 * @param z2 
 */
void SDL_DrawGPUDbgLine3D(
    const float x1,
    const float y1,
    const float z1,
    const float x2,
    const float y2,
    const float z2);

#ifdef SDL3_GPU_DBG_DRAW_IMPL

#include <stddef.h>
#include <string.h>

typedef enum
{
    PASS_TYPE_NONE,
    PASS_TYPE_2D,
    PASS_TYPE_3D,
}
pass_type_t;

static SDL_GPUDevice* device;
static SDL_GPUCommandBuffer* command_buffer;
static SDL_GPUGraphicsPipeline* line_graphics_pipeline_3d;
static SDL_GPUGraphicsPipeline* graphics_pipeline;
static SDL_GPURenderPass* render_pass;
static pass_type_t pass_type;
static float matrix[16];
static float channels[4];

bool SDL_InitGPUDbgDraw(
    SDL_GPUDevice* device_,
    const SDL_GPUTextureFormat color_texture_format,
    const SDL_GPUTextureFormat depth_texture_format)
{
    if (!device_)
    {
        return false;
    }
    device = device_;
    return true;
}

void SDL_QuitGPUDbgDraw()
{
    if (!device)
    {
        return;
    }
    device = NULL;
}

bool SDL_BeginGPUDbgPass3D(
    SDL_GPUCommandBuffer* command_buffer_,
    SDL_GPUTexture* depth_texture,
    SDL_GPUTexture* color_texture,
    float* matrix_)
{
    if (!device ||
        !command_buffer_ ||
        !depth_texture ||
        !color_texture ||
        !matrix_)
    {
        return false;
    }
    command_buffer = command_buffer_;
    memcpy(matrix, matrix_, sizeof(matrix));
    SDL_GPUColorTargetInfo color_target_info = {0};
    color_target_info.load_op = SDL_GPU_LOADOP_LOAD;
    color_target_info.store_op = SDL_GPU_STOREOP_STORE;
    color_target_info.texture = color_texture;
    SDL_GPUDepthStencilTargetInfo depth_stencil_target_info = {0};
    depth_stencil_target_info.load_op = SDL_GPU_LOADOP_LOAD;
    depth_stencil_target_info.store_op = SDL_GPU_STOREOP_STORE;
    depth_stencil_target_info.texture = depth_texture;
    render_pass = SDL_BeginGPURenderPass(
        command_buffer,
        &color_target_info, 1,
        &depth_stencil_target_info);
    if (!render_pass)
    {
        return false;
    }
    pass_type = PASS_TYPE_3D;
    SDL_PushGPUDebugGroup(command_buffer, "gpu_dbg_pass_3d");
    return true;
}

void SDL_EndGPUDbgPass()
{
    if (graphics_pipeline)
    {
        SDL_PopGPUDebugGroup(command_buffer);
    }
    SDL_EndGPURenderPass(render_pass);
    SDL_PopGPUDebugGroup(command_buffer);
    command_buffer = NULL;
    graphics_pipeline = NULL;
    render_pass = NULL;
    pass_type = PASS_TYPE_NONE;
}

void SDL_SetGPUDbgColor(
    const float red,
    const float green,
    const float blue,
    const float alpha)
{
    channels[0] = red;
    channels[1] = green;
    channels[2] = blue;
    channels[3] = alpha;
}

void SDL_DrawGPUDbgLine3D(
    const float x1,
    const float y1,
    const float z1,
    const float x2,
    const float y2,
    const float z2)
{
    if (graphics_pipeline != line_graphics_pipeline_3d)
    {
        graphics_pipeline = line_graphics_pipeline_3d;
        SDL_PushGPUDebugGroup(command_buffer, "gpu_dbg_line_3d");
        SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline);
    }
}

#endif /* SDL3_GPU_DBG_DRAW_IMPL */