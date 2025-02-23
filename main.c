#define SDL3_GPU_DBG_DRAW_IMPL
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "sdl3_gpu_dbg_draw.h"

int main(
    int argc,
    char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_Window* window = SDL_CreateWindow("sdl3_gpu_dbg_draw", 1024, 768, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!device)
    {
        SDL_Log("Failed to create device: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (!SDL_ClaimWindowForGPUDevice(device, window))
    {
        SDL_Log("Failed to create swapchain: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    const SDL_GPUTextureFormat color_texture_format = SDL_GetGPUSwapchainTextureFormat(device, window);
    const SDL_GPUTextureFormat depth_texture_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    if (!SDL_InitGPUDbgDraw(device, color_texture_format, depth_texture_format))
    {
        SDL_Log("Failed to initialize SDL GPU dbg draw: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_QuitGPUDbgDraw();
    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}