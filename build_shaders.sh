#!/bin/sh

PLATFORM=""
if [[ "$OSTYPE" == "msys" ]]; then
    PLATFORM="win64"
else
    echo "Missing build(s)"
    exit 1
fi

SHADERCROSS="bin/$PLATFORM/shadercross.exe"
INCLUDE="SDL_gpud_shaders.h"
SHADERS=("shader.frag" "shader.vert")

rm -f $INCLUDE
for FILE in "${SHADERS[@]}"; do
    SRC="shaders/$FILE"
    SPV="shaders/$FILE.spv"
    DXIL="shaders/$FILE.dxil"
    MSL="shaders/$FILE.msl"
    glslc "$SRC" -o "$SPV" -I src
    if [[ $? -ne 0 ]]; then
        echo "Error compiling $SRC to $SPV"
        exit 1
    fi
    $SHADERCROSS "$SPV" -o "$DXIL"
    if [[ $? -ne 0 ]]; then
        echo "Error converting $SPV to $DXIL"
        exit 1
    fi
    $SHADERCROSS "$SPV" -o "$MSL"
    if [[ $? -ne 0 ]]; then
        echo "Error converting $SPV to $MSL"
        exit 1
    fi
    xxd -i $SPV >> $INCLUDE
    xxd -i $DXIL >> $INCLUDE
    xxd -i $MSL >> $INCLUDE
done

sed -i 's/shaders_//g' $INCLUDE
sed -i 's/\bunsigned\b/static const unsigned/g' $INCLUDE