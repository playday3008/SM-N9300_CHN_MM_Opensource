################################################################################

1. How to Build
	- get Toolchain
		From android git server , codesourcery and etc ..
		 - aarch64-linux-android-4.9

	- get Toolchain
		Extract the attached file 'gcc-jopp-only.tar.gz' into your build machine

	- make output folder
		EX)OUTPUT_DIR=out
		$ mkdir out

	- edit Makefile
		
		edit "REAL_CC" with the gcc full path you extracted 'gcc-jopp-only.tar.gz'
			ex> REAL_CC = (directory where you downloaded 'gcc-jopp-only.tar.gz')/gcc-jopp-only/toolchains/linux-x86_64/aarch64-linux-android-4.9/prebuilt/bin/
                     aarch64-linux-android-gcc
			
		edit "CROSS_COMPILE" to right toolchain path(You downloaded).
		  EX)  CROSS_COMPILE= $(android platform directory you download)/android/prebuilt/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-
          Ex)  CROSS_COMPILE=/usr/local/toolchain/aarch64-linux-android-4.9/bin/aarch64-linux-android-          // check the location of toolchain
  	
		$ export ARCH=arm64		
		$ make -C $(pwd) O=$(pwd)/out KCFLAGS=-mno-android graceqlte_chnzc_defconfig
		$ make -C $(pwd) O=$(pwd)/out KCFLAGS=-mno-android

2. Output files
	- Kernel : out/arch/arm64/boot/Image
	- module : out/drivers/*/*.ko

3. How to Clean	
		Change to OUTPUT_DIR folder
		EX) $(pwd)/out
		$ make clean
################################################################################
