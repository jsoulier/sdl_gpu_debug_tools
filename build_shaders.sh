#!/bin/sh

local PLATFORM=""
if [[ "$OSTYPE" == "msys" ]]; then
    PLATFORM="win64"
else
    echo "Missing build for SDL_shadercross"
    exit 1
fi

SHADERCROSS="SDL_shadercross/$PLATFORM/shadercross.exe"
SHADERS="sdl3_gpu_dbg_draw_shaders.h"

create_shader() {
    local FILE=$1
    local SRC="shaders/$FILE"
    local SPV="shaders/bin/$FILE.spv"
    local DXIL="shaders/bin/$FILE.dxil"
    local MSL="shaders/bin/$FILE.msl"
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
    xxd -i $SPV >> $SHADERS
    xxd -i $DXIL >> $SHADERS
    xxd -i $MSL >> $SHADERS
}

shaders=("line_3d.frag" "line_3d.vert")
echo "#pragma once" > $SHADERS
for shader in "${shaders[@]}"; do
    ( create_shader "$shader" )
done
