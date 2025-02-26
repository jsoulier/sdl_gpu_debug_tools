#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <cstdint>
#include <cstdlib>
#include "sdl_gpud.h"

#define SPEED 0.5f
#define SENSITIVITY 0.002f
#define FOV 60.0f

int main(int argc, char** argv)
{
    SDL_Window* window = NULL;
    SDL_GPUDevice* device = NULL;
    SDL_GPUTexture* depth_texture = NULL;
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (!(window = SDL_CreateWindow("sdl_gpud", 1024, 768, SDL_WINDOW_RESIZABLE))) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (!(device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL))) {
        SDL_Log("Failed to create device: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("Failed to create swapchain: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (!SDL_InitGPUD(device, SDL_GetGPUSwapchainTextureFormat(device, window), SDL_GPU_TEXTUREFORMAT_D32_FLOAT)) {
        SDL_Log("Failed to initialize SDL GPUD: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    glm::vec3 position{};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 forward{0.0f, 0.0f, 1.0f};
    glm::vec3 right{1.0f, 0.0f, 0.0f};
    float width = 0.0f;
    float height = 0.0f;
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (!SDL_GetWindowRelativeMouseMode(window)) {
                    SDL_SetWindowRelativeMouseMode(window, true);
                }
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    SDL_SetWindowRelativeMouseMode(window, false);
                }
                break;
            case SDL_EVENT_MOUSE_MOTION:
                if (!SDL_GetWindowRelativeMouseMode(window)) {
                    break;
                }
                forward = glm::rotate(forward, -event.motion.yrel * SENSITIVITY, right);
                forward = glm::rotate(forward, -event.motion.xrel * SENSITIVITY, up);
                forward = glm::normalize(forward);
                right = glm::normalize(glm::cross(forward, up));
                break;
            }
        }
        SDL_WaitForGPUSwapchain(device, window);
        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
        if (!command_buffer) {
            SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
            continue;
        }
        SDL_GPUTexture* color_texture;
        uint32_t w;
        uint32_t h;
        if (!SDL_AcquireGPUSwapchainTexture(command_buffer, window, &color_texture, &w, &h)) {
            SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
            SDL_CancelGPUCommandBuffer(command_buffer);
            continue;
        }
        if (!color_texture || !w || !h) {
            SDL_SubmitGPUCommandBuffer(command_buffer);
            continue;
        }
        if (width != w || height != h) {
            SDL_PropertiesID depth_texture_props = SDL_CreateProperties();
            SDL_SetFloatProperty(depth_texture_props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, 1.0f);
            SDL_GPUTextureCreateInfo texture_info{};
            texture_info.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
            texture_info.type = SDL_GPU_TEXTURETYPE_2D;
            texture_info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
            texture_info.width = w;
            texture_info.height = h;
            texture_info.layer_count_or_depth = 1;
            texture_info.num_levels = 1;
            texture_info.props = depth_texture_props;
            depth_texture = SDL_CreateGPUTexture(device, &texture_info);
            SDL_DestroyProperties(depth_texture_props);
            if (!depth_texture) {
                SDL_Log("Failed to create texture: %s", SDL_GetError());
                SDL_SubmitGPUCommandBuffer(command_buffer);
                continue;
            }
            width = w;
            height = h;
        }
        SDL_GPUColorTargetInfo color_info{};
        color_info.texture = color_texture;
        color_info.load_op = SDL_GPU_LOADOP_CLEAR;
        color_info.store_op = SDL_GPU_STOREOP_STORE;
        SDL_GPUDepthStencilTargetInfo depth_info{};
        depth_info.texture = depth_texture;
        depth_info.load_op = SDL_GPU_LOADOP_CLEAR;
        depth_info.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
        depth_info.store_op = SDL_GPU_STOREOP_STORE;
        depth_info.clear_depth = 1.0f;
        depth_info.cycle = true;
        SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &color_info, 1, &depth_info);
        if (!render_pass) {
            SDL_Log("Failed to begin render pass: %s", SDL_GetError());
            SDL_SubmitGPUCommandBuffer(command_buffer);
            continue;
        }
        SDL_EndGPURenderPass(render_pass);
        const bool* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_W]) {
            position += forward * SPEED;
        }
        if (keys[SDL_SCANCODE_S]) {
            position -= forward * SPEED;
        }
        if (keys[SDL_SCANCODE_A]) {
            position -= right * SPEED;
        }
        if (keys[SDL_SCANCODE_D]) {
            position += right * SPEED;
        }
        if (keys[SDL_SCANCODE_E]) {
            position += up * SPEED;
        }
        if (keys[SDL_SCANCODE_Q]) {
            position -= up * SPEED;
        }
        const glm::mat4 view = glm::lookAt(position, position + forward, up);
        const glm::mat4 perspective = glm::perspective(glm::radians(FOV), width / height, 1.0f, 1000.0f);
        const glm::mat4 matrix_3d = perspective * view;
        const glm::mat4 matrix_2d = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
        SDL_FColor colors[] = {
            {1.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
        };
        for (int i = 0; i < SDL_arraysize(colors); i++) {
            const float y = i * 2.0f + 2.0f;
            SDL_SetGPUDColor(&colors[i]);
            SDL_DrawGPUDLine({0.0f, y}, {width, y});
            SDL_DrawGPUDLine({0.0f, height - y}, {width, height - y});
        }
        SDL_SetGPUDColor({1.0f, 0.0f, 0.0f, 0.2f});
        SDL_DrawGPUDPoint({30.0f, height - 50.0f}, 20.0f);
        SDL_SetGPUDColor({0.0f, 1.0f, 0.0f, 0.2f});
        SDL_DrawGPUDPoint({40.0f, height - 60.0f}, 20.0f);
        SDL_SetGPUDColor({1.0f, 0.0f, 1.0f, 1.0f});
        SDL_DrawGPUDText("abcdefghijklmnopqrstuvwxyz", 10.0f, 30.0f, 10);
        SDL_SetGPUDColor({0.0f, 1.0f, 1.0f, 1.0f});
        SDL_DrawGPUDText("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 10.0f, 45.0f, 10);
        SDL_SetGPUDColor({1.0f, 1.0f, 0.0f, 1.0f});
        SDL_DrawGPUDText("0123456789", 10.0f, 60.0f, 10);
        SDL_SetGPUDColor({1.0f, 1.0f, 1.0f, 1.0f});
        SDL_DrawGPUDText("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~", 10.0f, 75.0f, 10);
        SDL_SubmitGPUD(command_buffer, color_texture, NULL, &matrix_2d);
        SDL_SetGPUDColor({1.0f, 1.0f, 1.0f, 1.0f});
        const int grid = 10;
        const float spacing = 10.0f;
        for (int i = -grid; i <= grid; i++) {
            const float x = i * spacing;
            const float z = grid * spacing;
            SDL_DrawGPUDLine({x, 0.0f, -z}, {x, 0.0f, z});
        }
        for (int i = -grid; i <= grid; i++) {
            const float z = i * spacing;
            const float x = grid * spacing;
            SDL_DrawGPUDLine({-x, 0.0f, z}, {x, 0.0f, z});
        }
        SDL_SetGPUDColor({0.0f, 0.0f, 1.0f, 1.0f});
        SDL_DrawGPUDBox({10.0f, 10.0f, 10.0f}, {20.0f, 20.0f, 20.0f});
        SDL_SetGPUDColor({0.0f, 1.0f, 0.0f, 1.0f});
        SDL_DrawGPUDBox({10.0f, 10.0f, 20.0f}, {20.0f, 20.0f, 30.0f});
        SDL_SetGPUDColor({1.0f, 1.0f, 1.0f, 1.0f});
        SDL_DrawGPUDPoint({10.0f, 10.0f, -10.0f}, 0.2f);
        SDL_SetGPUDColor({1.0f, 0.0f, 0.0f, 1.0f});
        SDL_DrawGPUDLine({10.0f, 10.0f, -10.0f}, {20.0f, 10.0f, -10.0f});
        SDL_SetGPUDColor({0.0f, 1.0f, 0.0f, 1.0f});
        SDL_DrawGPUDLine({10.0f, 10.0f, -10.0f}, {10.0f, 20.0f, -10.0f});
        SDL_SetGPUDColor({0.0f, 0.0f, 1.0f, 1.0f});
        SDL_DrawGPUDLine({10.0f, 10.0f, -10.0f}, {10.0f, 10.0f, -20.0f});
        SDL_SubmitGPUD(command_buffer, color_texture, depth_texture, &matrix_3d);
        SDL_SubmitGPUCommandBuffer(command_buffer);
    }
    SDL_QuitGPUD();
    SDL_ReleaseGPUTexture(device, depth_texture);
    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}