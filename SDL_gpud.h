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
#endif /* ifdef __cplusplus */

/**
 * @brief 
 */
typedef struct
{
    float x;
    float y;
    float z;
    Uint32 color; /**< Internal */
} SDL_GPUDVertex;

/**
 * @brief 
 * @param device 
 * @param color_format 
 * @param depth_format 
 * @return 
 */
bool SDL_InitGPUD(
    SDL_GPUDevice* device,
    const SDL_GPUTextureFormat color_format,
    const SDL_GPUTextureFormat depth_format);

/**
 * @brief 
 */
void SDL_QuitGPUD();

/**
 * @brief 
 * @param color 
 */
void SDL_SetGPUDColor(
    const SDL_FColor* color);

/**
 * @brief 
 * @param center
 * @param radius 
 */
void SDL_DrawGPUDPoint(
    const SDL_GPUDVertex* center,
    const float radius);

/**
 * @brief 
 * @param start 
 * @param end
 */
void SDL_DrawGPUDBox(
    const SDL_GPUDVertex* start,
    const SDL_GPUDVertex* end);

/**
 * @brief 
 * @param start 
 * @param end 
 */
void SDL_DrawGPUDLine(
    const SDL_GPUDVertex* start,
    const SDL_GPUDVertex* end);

/**
 * @brief 
 * @param center
 * @param radius 
 */
void SDL_DrawGPUDSphere(
    const SDL_GPUDVertex* center,
    const float radius);

/**
 * @brief 
 * @param x 
 * @param y 
 * @param size 
 */
void SDL_DrawGPUDText(
    const char* text,
    const float x,
    const float y,
    const int size);

/**
 * @brief 
 * @param command_buffer 
 * @param color_texture 
 * @param depth_texture 
 * @param matrix 
 */
void SDL_SubmitGPUD(
    SDL_GPUCommandBuffer* command_buffer,
    SDL_GPUTexture* color_texture,
    SDL_GPUTexture* depth_texture,
    const void* matrix);

#ifdef __cplusplus
} /* extern "C" */
#endif /* ifdef __cplusplus */
#ifdef __cplusplus

/**
 * @brief 
 * @param color 
 */
inline void SDL_SetGPUDColor(
    const SDL_FColor& color)
{
    SDL_SetGPUDColor(&color);
}

/**
 * @brief 
 * @param center 
 * @param radius 
 */
inline void SDL_DrawGPUDPoint(
    const SDL_GPUDVertex& center,
    const float radius)
{
    SDL_DrawGPUDPoint(&center, radius);
}

/**
 * @brief 
 * @param start 
 * @param end 
 */
inline void SDL_DrawGPUDBox(
    const SDL_GPUDVertex& start,
    const SDL_GPUDVertex& end)
{
    SDL_DrawGPUDBox(&start, &end);
}

/**
 * @brief 
 * @param start 
 * @param end 
 */
inline void SDL_DrawGPUDLine(
    const SDL_GPUDVertex& start,
    const SDL_GPUDVertex& end)
{
    SDL_DrawGPUDLine(&start, &end);
}

/**
 * @brief 
 * @param center 
 * @param radius 
 */
inline void SDL_DrawGPUDSphere(
    const SDL_GPUDVertex& center,
    const float radius)
{
    SDL_DrawGPUDSphere(&center, radius);
}

#endif /* ifdef __cplusplus */
#endif /* ifndef SDL_GPUD_H */
#ifdef SDL_GPUD_IMPL

#include "SDL_gpud_shaders.h"

#define BUFFER_CAPACITY 1024
#define SPHERE_VERTICES 20

typedef enum
{
    COMMAND_TYPE_LINE,
    COMMAND_TYPE_POLY,
} CommandType;

typedef struct Command Command;
typedef struct Command
{
    Command* next;
    CommandType type;
    SDL_GPUTransferBuffer* transfer_buffer;
    SDL_GPUBuffer* buffer;
    Uint32 size;
    Uint32 capacity;
    Uint8* data;
} Command;

static SDL_GPUDevice* device;
static SDL_GPUGraphicsPipeline* line_2d_pipeline;
static SDL_GPUGraphicsPipeline* line_3d_pipeline;
static SDL_GPUGraphicsPipeline* poly_2d_pipeline;
static SDL_GPUGraphicsPipeline* poly_3d_pipeline;
static Uint32 color;
static Command* head;
static Command* tail;

bool SDL_InitGPUD(
    SDL_GPUDevice* handle,
    const SDL_GPUTextureFormat color_format,
    const SDL_GPUTextureFormat depth_format)
{
    if (device) {
        SDL_QuitGPUD();
    }
    device = handle;
    if (!device) {
        return SDL_InvalidParamError("device");
    }
    SDL_GPUShaderCreateInfo fragment_shader_info = {0};
    SDL_GPUShaderCreateInfo vertex_shader_info = {0};
    if (SDL_GetGPUShaderFormats(device) & SDL_GPU_SHADERFORMAT_SPIRV) {
        fragment_shader_info.code = shader_frag_spv;
        vertex_shader_info.code = shader_vert_spv;
        fragment_shader_info.code_size = shader_frag_spv_len;
        vertex_shader_info.code_size = shader_vert_spv_len;
        fragment_shader_info.entrypoint = "main";
        vertex_shader_info.entrypoint = "main";
        fragment_shader_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
        vertex_shader_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    } else if (SDL_GetGPUShaderFormats(device) & SDL_GPU_SHADERFORMAT_DXIL) {
        fragment_shader_info.code = shader_frag_dxil;
        vertex_shader_info.code = shader_vert_dxil;
        fragment_shader_info.code_size = shader_frag_dxil_len;
        vertex_shader_info.code_size = shader_vert_dxil_len;
        fragment_shader_info.entrypoint = "main";
        vertex_shader_info.entrypoint = "main";
        fragment_shader_info.format = SDL_GPU_SHADERFORMAT_DXIL;
        vertex_shader_info.format = SDL_GPU_SHADERFORMAT_DXIL;
    } else if (SDL_GetGPUShaderFormats(device) & SDL_GPU_SHADERFORMAT_MSL) {
        fragment_shader_info.code = shader_frag_msl;
        vertex_shader_info.code = shader_vert_msl;
        fragment_shader_info.code_size = shader_frag_msl_len;
        vertex_shader_info.code_size = shader_vert_msl_len;
        fragment_shader_info.entrypoint = "main0";
        vertex_shader_info.entrypoint = "main0";
        fragment_shader_info.format = SDL_GPU_SHADERFORMAT_MSL;
        vertex_shader_info.format = SDL_GPU_SHADERFORMAT_MSL;
    } else {
        return SDL_Unsupported();
    }
    fragment_shader_info.num_uniform_buffers = 0;
    vertex_shader_info.num_uniform_buffers = 1;
    fragment_shader_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    vertex_shader_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    SDL_GPUShader* fragment_shader = SDL_CreateGPUShader(device, &fragment_shader_info);
    SDL_GPUShader* vertex_shader = SDL_CreateGPUShader(device, &vertex_shader_info);
    if (!fragment_shader || !vertex_shader) {
        goto error;
    }
    SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                .format = color_format,
                .blend_state = {
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
        .vertex_input_state = {
            .num_vertex_attributes = 2,
            .vertex_attributes = (SDL_GPUVertexAttribute[]) {{
                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                .location = 0,
                .offset = 0,
            }, {
                .format = SDL_GPU_VERTEXELEMENTFORMAT_UINT,
                .location = 1,
                .offset = 12,
            }},
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]) {{
                .pitch = 16,
            }},
        },
        .depth_stencil_state = {
            .compare_op = SDL_GPU_COMPAREOP_LESS,
            .enable_depth_test = true,
            .enable_depth_write = true,
        }
    };
    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
    pipeline_info.target_info.has_depth_stencil_target = false;
    line_2d_pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
    pipeline_info.target_info.has_depth_stencil_target = true;
    line_3d_pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeline_info.target_info.has_depth_stencil_target = false;
    poly_2d_pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeline_info.target_info.has_depth_stencil_target = true;
    poly_3d_pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    if (!line_2d_pipeline || !line_3d_pipeline || !poly_2d_pipeline || !poly_3d_pipeline) {
        goto error;
    }
    bool status = true;
    goto success;
error:
    status = false;
success:
    SDL_ReleaseGPUShader(device, fragment_shader);
    SDL_ReleaseGPUShader(device, vertex_shader);
    if (!status) {
        SDL_QuitGPUD();
    }
    return true;
}

void SDL_QuitGPUD()
{
    if (!device) {
        return;
    }
    for (Command* command = head; command;) {
        SDL_ReleaseGPUTransferBuffer(device, command->transfer_buffer);
        SDL_ReleaseGPUBuffer(device, command->buffer);
        Command* prev = command;
        command = command->next;
        free(prev);
    }
    head = NULL;
    tail = NULL;
    SDL_ReleaseGPUGraphicsPipeline(device, line_2d_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, line_3d_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, poly_2d_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, poly_3d_pipeline);
    line_2d_pipeline = NULL;
    line_3d_pipeline = NULL;
    poly_2d_pipeline = NULL;
    poly_3d_pipeline = NULL;
    device = NULL;
}

void SDL_SetGPUDColor(
    const SDL_FColor* handle)
{
    if (!device) {
        return;
    }
    if (!handle) {
        SDL_InvalidParamError("handle");
        return;
    }
    const Uint8 red = SDL_min(handle->r * UINT8_MAX, UINT8_MAX);
    const Uint8 green = SDL_min(handle->g * UINT8_MAX, UINT8_MAX);
    const Uint8 blue = SDL_min(handle->b * UINT8_MAX, UINT8_MAX);
    const Uint8 alpha = SDL_min(handle->a * UINT8_MAX, UINT8_MAX);
    color = red << 24 | green << 16 | blue << 8 | alpha;
}

static void PushCommand(
    const CommandType type,
    void* data,
    const Uint32 size)
{
    Command* command = tail;
    if (!command || command->type != type || command->size + size > command->capacity) {
        command = SDL_malloc(sizeof(Command));
        command->next = NULL;
        command->type = type;
        command->size = 0;
        command->capacity = SDL_max(BUFFER_CAPACITY, size);
        command->data = NULL;
        SDL_GPUBufferCreateInfo buffer_info = {0};
        buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        buffer_info.size = command->capacity;
        command->buffer = SDL_CreateGPUBuffer(device, &buffer_info);
        SDL_GPUTransferBufferCreateInfo transfer_buffer_info = {0};
        transfer_buffer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transfer_buffer_info.size = command->capacity;
        command->transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_buffer_info);
        command->data = SDL_MapGPUTransferBuffer(device, command->transfer_buffer, false);
        if (!command->data || !command->buffer) {
            SDL_ReleaseGPUBuffer(device, command->buffer);
            SDL_ReleaseGPUTransferBuffer(device, command->transfer_buffer);
            SDL_free(command);
            return;
        }
        if (tail) {
            tail->next = command;
        }
        tail = command;
        if (!head) {
            head = command;
        }
    }
    SDL_memcpy(command->data + command->size, data, size);
    command->size += size;
}

void SDL_DrawGPUDPoint(
    const SDL_GPUDVertex* center,
    const float radius)
{
    if (!device) {
        return;
    }
    if (!center) {
        SDL_InvalidParamError("center");
        return;
    }
    if (radius < SDL_FLT_EPSILON) {
        SDL_InvalidParamError("radius");
        return;
    }
    SDL_GPUDVertex start = {0};
    SDL_GPUDVertex end = {0};
    start.x = center->x - radius;
    start.y = center->y - radius;
    start.z = center->z - radius;
    end.x = center->x + radius;
    end.y = center->y + radius;
    end.z = center->z + radius;
    SDL_GPUDVertex vertices[36] = {
        {start.x, start.y, end.z, color},
        {end.x, start.y, end.z, color},
        {end.x, end.y, end.z, color},
        {start.x, start.y, end.z, color},
        {end.x, end.y, end.z, color},
        {start.x, end.y, end.z, color},
        {start.x, start.y, start.z, color},
        {start.x, end.y, start.z, color},
        {end.x, end.y, start.z, color},
        {start.x, start.y, start.z, color},
        {end.x, end.y, start.z, color},
        {end.x, start.y, start.z, color},
        {start.x, start.y, start.z, color},
        {start.x, start.y, end.z, color},
        {start.x, end.y, end.z, color},
        {start.x, start.y, start.z, color},
        {start.x, end.y, end.z, color},
        {start.x, end.y, start.z, color},
        {end.x, start.y, start.z, color},
        {end.x, end.y, start.z, color},
        {end.x, end.y, end.z, color},
        {end.x, start.y, start.z, color},
        {end.x, end.y, end.z, color},
        {end.x, start.y, end.z, color},
        {start.x, end.y, start.z, color},
        {start.x, end.y, end.z, color},
        {end.x, end.y, end.z, color},
        {start.x, end.y, start.z, color},
        {end.x, end.y, end.z, color},
        {end.x, end.y, start.z, color},
        {start.x, start.y, start.z, color},
        {end.x, start.y, start.z, color},
        {end.x, start.y, end.z, color},
        {start.x, start.y, start.z, color},
        {end.x, start.y, end.z, color},
        {start.x, start.y, end.z, color},
    };
    PushCommand(COMMAND_TYPE_POLY, vertices, sizeof(vertices));
}

void SDL_DrawGPUDBox(
    const SDL_GPUDVertex* start,
    const SDL_GPUDVertex* end)
{
    if (!device) {
        return;
    }
    if (!start) {
        SDL_InvalidParamError("start");
        return;
    }
    if (!end) {
        SDL_InvalidParamError("end");
        return;
    }
    SDL_GPUDVertex vertices[24] = {
        {start->x, start->y, start->z, color},
        {end->x, start->y, start->z, color},
        {end->x, start->y, start->z, color},
        {end->x, start->y, end->z, color},
        {end->x, start->y, end->z, color},
        {start->x, start->y, end->z, color},
        {start->x, start->y, end->z, color},
        {start->x, start->y, start->z, color},
        {start->x, end->y, start->z, color},
        {end->x, end->y, start->z, color},
        {end->x, end->y, start->z, color},
        {end->x, end->y, end->z, color},
        {end->x, end->y, end->z, color},
        {start->x, end->y, end->z, color},
        {start->x, end->y, end->z, color},
        {start->x, end->y, start->z, color},
        {start->x, start->y, start->z, color},
        {start->x, end->y, start->z, color},
        {end->x, start->y, start->z, color},
        {end->x, end->y, start->z, color},
        {end->x, start->y, end->z, color},
        {end->x, end->y, end->z, color},
        {start->x, start->y, end->z, color},
        {start->x, end->y, end->z, color},
    };
    PushCommand(COMMAND_TYPE_LINE, vertices, sizeof(vertices));
}

void SDL_DrawGPUDLine(
    const SDL_GPUDVertex* start,
    const SDL_GPUDVertex* end)
{
    if (!device) {
        return;
    }
    if (!start) {
        SDL_InvalidParamError("start");
        return;
    }
    if (!end) {
        SDL_InvalidParamError("end");
        return;
    }
    SDL_GPUDVertex vertices[2];
    vertices[0] = *start;
    vertices[1] = *end;
    vertices[0].color = color;
    vertices[1].color = color;
    PushCommand(COMMAND_TYPE_LINE, vertices, sizeof(vertices));
}

void SDL_DrawGPUDSphere(
    const SDL_GPUDVertex* center,
    const float radius)
{
    if (!device) {
        return;
    }
    if (!center) {
        SDL_InvalidParamError("center");
        return;
    }
    if (radius < SDL_FLT_EPSILON) {
        SDL_InvalidParamError("radius");
        return;
    }
    SDL_GPUDVertex vertices[SPHERE_VERTICES * SPHERE_VERTICES * 2];
    int index = 0;
    for (int i = 0; i < SPHERE_VERTICES; i++) {
        const float phi = i / (float) (SPHERE_VERTICES - 1) * SDL_PI_F;
        for (int j = 0; j < SPHERE_VERTICES; j++) {
            const float theta = j / (float) (SPHERE_VERTICES - 1) * 2 * SDL_PI_F;
            vertices[index].x = center->x + radius * SDL_sin(phi) * SDL_cos(theta);
            vertices[index].y = center->y + radius * SDL_sin(phi) * SDL_sin(theta);
            vertices[index].z = center->z + radius * SDL_cos(phi);
            vertices[index].color = center->color;
            index++;
        }
    }
    for (int i = 0; i < SPHERE_VERTICES - 1; i++) {
        for (int j = 0; j < SPHERE_VERTICES; j++) {
            const int curr = i * SPHERE_VERTICES + j;
            const int next = ((i + 1) * SPHERE_VERTICES + j) % (SPHERE_VERTICES * SPHERE_VERTICES);
            SDL_DrawGPUDLine(&vertices[curr], &vertices[next]);
        }
    }
    for (int i = 0; i < SPHERE_VERTICES - 1; i++) {
        for (int j = 0; j < SPHERE_VERTICES - 1; j++) {
            const int curr = i * SPHERE_VERTICES + j;
            const int next = i * SPHERE_VERTICES + (j + 1);
            SDL_DrawGPUDLine(&vertices[curr], &vertices[next]);
        }
    }
}

static void TextFunc(
    const int x1,
    const int y1,
    const int x2,
    const int y2)
{
    SDL_GPUDVertex start = {x1, y1, 0.0f};
    SDL_GPUDVertex end = {x2, y2, 0.0f};
    SDL_DrawGPUDLine(&start, &end);
}

/* Modified from: https://github.com/gamelly/gly-type */
void SDL_DrawGPUDText(
    const char* text,
    const float x,
    const float y,
    const int size)
{
    if (!device) {
        return;
    }
    if (!text) {
        SDL_InvalidParamError("text");
        return;
    }
    if (size < 3) {
        SDL_InvalidParamError("size");
        return;
    }
    static const unsigned char segments_1[] = {
        0x00, 0x28, 0x81, 0x13, 0xbb, 0x42, 0x33, 0x80, 0x12, 0x21, 0x00, 0x09,
        0x20, 0x00, 0x80, 0x00, 0x7e, 0x30, 0x76, 0x3e, 0x8c, 0xb9, 0xf9, 0x0f,
        0xff, 0xbf, 0xc0, 0x60, 0x00, 0x03, 0x00, 0x00, 0xf7, 0xcf, 0xf9, 0xf3,
        0xe1, 0xf3, 0xc3, 0xfb, 0xcc, 0x33, 0x7c, 0xc0, 0xf0, 0xcf, 0xcc, 0xff,
        0xc7, 0x8f, 0xc7, 0xbb, 0x03, 0xfc, 0x84, 0xfc, 0x00, 0x84, 0x33, 0xe1,
        0x00, 0x1e, 0x00, 0x30, 0x00, 0x70, 0xe0, 0xf1, 0x1c, 0xe1, 0xc1, 0xe1,
        0xc0, 0xc0, 0x60, 0xc0, 0xe0, 0xc9, 0xc1, 0xe1, 0xc1, 0x0e, 0xc0, 0xa1,
        0x03, 0xe0, 0x04, 0xe4, 0x00, 0x00, 0x16, 0x33, 0x00, 0x33, 0x85
    };
    static const unsigned char segments_2[] = {
        0x00, 0x80, 0x00, 0x80, 0x07, 0x80, 0x4a, 0x00, 0xc8, 0xb0, 0x7b, 0x80,
        0x80, 0x03, 0x80, 0x50, 0x88, 0x8c, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00,
        0x03, 0x03, 0x80, 0x80, 0xc8, 0x80, 0x48, 0x9e, 0x06, 0x03, 0x07, 0x00,
        0xb0, 0x03, 0x03, 0x02, 0x03, 0x04, 0x00, 0x31, 0x00, 0x04, 0x28, 0x00,
        0x03, 0x03, 0x23, 0x03, 0x04, 0x00, 0xe0, 0x04, 0x78, 0x07, 0x50, 0x00,
        0x28, 0x00, 0x98, 0x00, 0x08, 0x0d, 0x05, 0x00, 0x06, 0x01, 0x01, 0xa2,
        0x05, 0x00, 0x04, 0x8d, 0x00, 0x94, 0x04, 0x04, 0x05, 0x06, 0x88, 0xc0,
        0x04, 0x04, 0xa4, 0xa4, 0x78, 0x58, 0xa0, 0x49, 0x04, 0x32, 0x06
    };
    unsigned char c, m1, m2, segment;
    int sp2, sm1, x1, x2, x3, y1, y2, y3;
    int sd4, x2m1, x2p1, y2m1, y2p1, sne1;
    sp2 = size + 2;
    sm1 = size - 1;
    sd4 = size / 4;
    sne1 = ~size & 1;
    x1 = x;
    y1 = y;
    y2 = y1 + (sm1 / 2);
    y3 = y1 + sm1;
    if (size < 0) {
        y3 = y3 ^ y1;
        y1 = y3 ^ y1;
        y3 = y3 ^ y1;
    }
    while (*text) {
        x2 = x1 + (sm1 / 2);
        x3 = x1 + sm1;
        c = *text - 0x20;
        if (c > (0x7f - 0x20)) {
            goto gly_type_skip_char;
        }
        m1 = segments_1[c];
        m2 = segments_2[c];
        if (m2 == 0x80) {
            segment = 0;
            x2m1 = x2 - sd4;
            x2p1 = x2 + sd4 + sne1;
            y2m1 = y2 - sd4 + sne1;
            y2p1 = y2 + sd4;
            while (segment < 8) {
                switch (m1 & (1 << segment) ? segment : 8) {
                case 0: TextFunc(x1, y2m1, x3, y2m1); break;
                case 1: TextFunc(x1, y2p1, x3, y2p1); break;
                case 2: TextFunc(x2, y2, x2, y3); break;
                case 3: TextFunc(x2, y1, x2, y2p1); break;
                case 4:
                    TextFunc(x2m1, y1, x2m1, y3);
                    TextFunc(x2p1, y1, x2p1, y3);
                    break;
                case 5: TextFunc(x2m1, y3, x2p1, y2p1); break;
                case 6:
                    TextFunc(x2p1, y2m1, x2p1, y1);
                    TextFunc(x2m1, y2m1, x2m1, y1);
                    TextFunc(x2m1, y2m1, x2p1, y2m1);
                    TextFunc(x2m1, y1, x2p1, y1);
                    break;
                case 7:
                    TextFunc(x2m1, y2p1, x2p1, y2p1);
                    TextFunc(x2m1, y2p1, x2m1, y3);
                    TextFunc(x2p1, y2p1, x2p1, y3);
                    TextFunc(x2m1, y3, x2p1, y3);
                    break;
                }
                segment++;
            }
            goto gly_type_next_char;
        }
        if ('a' <= *text && *text <= 'z' && !(m1 & 0xe1) && !(m2 & 0x49)) {
            x3 = x2;
            x2 = x1;
        }
        segment = 0;
        while (segment < 8) {
            switch (m1 & (1 << segment) ? segment : 8) {
            case 0: TextFunc(x1, y1, x2, y1); break;
            case 1: TextFunc(x2, y1, x3, y1); break;
            case 2: TextFunc(x3, y1, x3, y2); break;
            case 3: TextFunc(x3, y2, x3, y3); break;
            case 4: TextFunc(x2, y3, x3, y3); break;
            case 5: TextFunc(x1, y3, x2, y3); break;
            case 6: TextFunc(x1, y2, x1, y3); break;
            case 7: TextFunc(x1, y1, x1, y2); break;
            }
            segment++;
        }
        segment = 0;
        while (segment < 7) {
            switch (m2 & (1 << segment) ? segment : 7) {
            case 0: TextFunc(x1, y2, x2, y2); break;
            case 1: TextFunc(x2, y2, x3, y2); break;
            case 2:
                m2 & 0x3 ? TextFunc(x2, y2, x2, m1 & 0x03 ? y1 : y3)
                         : TextFunc(x2, y1, x2, y3);
                break;
            case 3:
                m2 & 0x80 ? TextFunc(x1, y2, x2, y1)
                          : TextFunc(x1, y1, x2, y2);
                break;
            case 4:
                m2 & 0x80 ? TextFunc(x2, y1, x3, y2)
                          : TextFunc(x2, y2, x3, y1);
                break;
            case 5:
                m2 & 0x80 ? TextFunc(x2, y3, x3, y2)
                          : TextFunc(x2, y2, x3, y3);
                break;
            case 6:
                m2 & 0x80 ? TextFunc(x1, y2, x2, y3)
                          : TextFunc(x1, y3, x2, y2);
                break;
            }
            segment++;
        }
    gly_type_next_char:
        x1 += sp2;
    gly_type_skip_char:
        text++;
    }
}

void SDL_SubmitGPUD(
    SDL_GPUCommandBuffer* command_buffer,
    SDL_GPUTexture* color_texture,
    SDL_GPUTexture* depth_texture,
    const void* matrix)
{
    if (!device) {
        return;
    }
    if (!command_buffer) {
        SDL_InvalidParamError("command_buffer");
        return;
    }
    if (!color_texture) {
        SDL_InvalidParamError("color_texture");
        return;
    }
    if (!matrix) {
        SDL_InvalidParamError("matrix");
        return;
    }
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    if (!copy_pass) {
        return;
    }
    for (Command* command = head; command; command = command->next) {
        SDL_UnmapGPUTransferBuffer(device, command->transfer_buffer);
        command->data = NULL;
        SDL_GPUTransferBufferLocation location = {0};
        SDL_GPUBufferRegion region = {0};
        location.transfer_buffer = command->transfer_buffer;
        region.buffer = command->buffer;
        region.size = command->size;
        SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);
    }
    SDL_EndGPUCopyPass(copy_pass);
    SDL_GPUColorTargetInfo color_info = {0};
    color_info.texture = color_texture;
    color_info.load_op = SDL_GPU_LOADOP_LOAD;
    color_info.store_op = SDL_GPU_STOREOP_STORE;
    SDL_GPURenderPass* render_pass;
    if (depth_texture) {
        SDL_GPUDepthStencilTargetInfo depth_info = {0};
        depth_info.texture = depth_texture;
        depth_info.load_op = SDL_GPU_LOADOP_LOAD;
        depth_info.store_op = SDL_GPU_STOREOP_STORE;
        render_pass = SDL_BeginGPURenderPass(command_buffer, &color_info, 1, &depth_info);
    } else {
        render_pass = SDL_BeginGPURenderPass(command_buffer, &color_info, 1, NULL);
    }
    if (!render_pass) {
        return;
    }
    SDL_GPUGraphicsPipeline* pipeline1 = NULL;
    SDL_GPUGraphicsPipeline* pipeline2 = NULL;
    for (Command* command = head; command;) {
        switch (command->type) {
        case COMMAND_TYPE_LINE:
            if (depth_texture) {
                pipeline2 = line_3d_pipeline;
            } else {
                pipeline2 = line_2d_pipeline;
            }
            break;
        case COMMAND_TYPE_POLY:
            if (depth_texture) {
                pipeline2 = poly_3d_pipeline;
            } else {
                pipeline2 = poly_2d_pipeline;
            }
            break;
        }
        if (pipeline1 != pipeline2) {
            pipeline1 = pipeline2;
            SDL_BindGPUGraphicsPipeline(render_pass, pipeline1);
            SDL_PushGPUVertexUniformData(command_buffer, 0, matrix, 16 * sizeof(float));
        }
        SDL_GPUBufferBinding binding = {0};
        binding.buffer = command->buffer;
        SDL_BindGPUVertexBuffers(render_pass, 0, &binding, 1);
        SDL_DrawGPUPrimitives(render_pass, command->size / (4 * sizeof(float)), 1, 0, 0);
        SDL_ReleaseGPUTransferBuffer(device, command->transfer_buffer);
        SDL_ReleaseGPUBuffer(device, command->buffer);
        Command* prev = command;
        command = command->next;
        free(prev);
    }
    SDL_EndGPURenderPass(render_pass);
    head = NULL;
    tail = NULL;
}

#endif /* ifdef SDL_GPU_IMPL */