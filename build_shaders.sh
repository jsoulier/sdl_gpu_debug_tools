#!/bin/sh

PLATFORM=""
if [[ "$OSTYPE" == "msys" ]]; then
    PLATFORM="win64"
else
    echo "Missing build for SDL_shadercross"
    exit 1
fi

SHADERCROSS="SDL_shadercross/$PLATFORM/shadercross.exe"
INCLUDE="sdl3_gpu_dbg_draw_shaders.h"
SHADERS=("line_3d.frag" "line_3d.vert")

echo "#pragma once" > $INCLUDE
for FILE in "${SHADERS[@]}"; do
    SRC="shaders/$FILE"
    SPV="shaders/bin/$FILE.spv"
    DXIL="shaders/bin/$FILE.dxil"
    MSL="shaders/bin/$FILE.msl"
    glslc "$SRC" -o "$SPV" -I src
    if [[ $? -ne 0 ]]; then
        echo "Error compiling shader $SRC to $SPV"
        exit 1
    fi
    $SHADERCROSS "$SPV" -o "$DXIL"
    if [[ $? -ne 0 ]]; then
        echo "Error converting $SPV to $DXIL."
        exit 1
    fi
    $SHADERCROSS "$SPV" -o "$MSL"
    if [[ $? -ne 0 ]]; then
        echo "Error converting $SPV to $MSL."
        exit 1
    fi
    xxd -i $SPV >> $INCLUDE
    xxd -i $DXIL >> $INCLUDE
    xxd -i $MSL >> $INCLUDE
done