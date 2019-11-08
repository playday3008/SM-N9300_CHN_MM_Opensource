#!/bin/bash

export ARCH=arm64

#export PATH=/opt/toolchains/aarch64-linux-android-4.9/bin:$PATH
export PATH=$(pwd)/../PLATFORM/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin:$PATH
#export PATH=$(pwd)/prebuilts/gcc-jopp-only/toolchains/linux-x86_64/aarch64-linux-android-4.9/prebuilt/bin:$PATH

mkdir out

make -C $(pwd) O=$(pwd)/out CROSS_COMPILE=aarch64-linux-android- KCFLAGS=-mno-android graceqlte_$1_defconfig
make -C $(pwd) O=$(pwd)/out CROSS_COMPILE=aarch64-linux-android- KCFLAGS=-mno-android


cp out/arch/arm64/boot/Image $(pwd)/arch/arm64/boot/Image