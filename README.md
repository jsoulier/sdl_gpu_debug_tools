# SDL GPUD

> NOTE: Still a work in progress. More shapes and text-rendering will be added soon

An embeddable immediate-mode drawing API using SDL3 GPU for C and C++.

### Using SDL GPUD

SDL GPUD consists of two headers:
- `sdl_gpud.h` (Implementation)
- `sdl_gpud_shaders.h` (Precompiled SPV, DXIL, and MSL shaders)

To use SDL GPUD, copy these two headers into your project and in **one C** file add:
```c
#define SDL_GPUD_IMPL
#include "sdl_gpud.h"
```

See the following for an example (or [main.cpp](main.cpp)):
```c++
if (!SDL_InitGPUD(device,
    SDL_GetGPUSwapchainTextureFormat(device, window),
    SDL_GPU_TEXTUREFORMAT_D32_FLOAT))
{
    // Handle errors
}

while (true)
{
    // Acquire command buffer and swapchain texture
    // Do regular drawing...

    // Draw north facing red line
    SDL_GPUDColor({1.0f, 0.0f, 0.0f, 1.0f});
    SDL_GPUDLine({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 100.0f});

    // Draw upwards facing green line
    SDL_GPUDColor({0.0f, 1.0f, 0.0f, 1.0f});
    SDL_GPUDLine({0.0f, 0.0f, 0.0f}, {0.0f, 100.0f, 0.0f});

    // Draw east facing blue line
    SDL_GPUDColor({0.0f, 0.0f, 1.0f, 1.0f});
    SDL_GPUDLine({0.0f, 0.0f, 0.0f}, {100.0f, 0.0f, 0.0f});

    // Submit draw commands
    SDL_GPUDraw(/* ... */);
    SDL_SubmitGPUCommandBuffer(/* ... */);
}

SDL_QuitGPUD();
```

### Attributions

- Sean Barrett (stb_easy_font)