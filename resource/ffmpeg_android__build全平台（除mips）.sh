#!/bin/bash
export NDK_HOME=/usr/work/ndk/android-ndk-r14b

function build
{
	echo "start build ffmpeg for $CPU"
	./configure --target-os=linux \
	--prefix=$PREFIX --arch=$CPU \
	--disable-doc \
	--enable-shared \
	--disable-static \
	--disable-yasm \
	--disable-asm \
	--disable-symver \
	--disable-encoders \
	--disable-programs \
 	--disable-htmlpages \
  	--disable-manpages \
  	--disable-podpages \
  	--disable-txtpages \
	--disable-muxers \
	--disable-ffmpeg \
	--disable-ffplay \
	--disable-ffprobe \
	--disable-avdevice \
	--disable-avfilter \
	--disable-debug \
	--cross-prefix=$CROSS_COMPILE \
	--enable-cross-compile \
	--sysroot=$SYSROOT \
	--enable-small \
	--enable-protocols \
	--extra-cflags="-Os -fpic $ADDI_CFLAGS" \
	--extra-ldflags="$ADDI_LDFLAGS" \
	$ADDITIONAL_CONFIGURE_FLAG
	make clean
	make
	make install
	echo "build ffmpeg for $CPU finished"
}

#arm
PLATFORM_VERSION=android-14
ARCH=arm
CPU=armeabi
PREFIX=$(pwd)/android_all/$CPU
TOOLCHAIN=$NDK_HOME/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/arm-linux-androideabi-
ADDI_CFLAGS="-marm -mthumb"
ADDI_LDFLAGS=""
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
build

#arm-v7a
PLATFORM_VERSION=android-14
ARCH=arm
CPU=armeabi-v7a
PREFIX=$(pwd)/android_all/$CPU
TOOLCHAIN=$NDK_HOME/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/arm-linux-androideabi-
ADDI_CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -mthumb -mfpu=neon"
ADDI_LDFLAGS="-march=armv7-a -Wl,--fix-cortex-a8"
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
build


#arm64
PLATFORM_VERSION=android-21
ARCH=arm64
CPU=arm64
PREFIX=$(pwd)/android_all/$CPU
TOOLCHAIN=$NDK_HOME/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/aarch64-linux-android-
ADDI_CFLAGS=""
ADDI_LDFLAGS=""
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
build


#x86
PLATFORM_VERSION=android-14
ARCH=x86
CPU=x86
PREFIX=$(pwd)/android_all/$CPU
TOOLCHAIN=$NDK_HOME/toolchains/x86-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/i686-linux-android-
ADDI_CFLAGS="-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"
ADDI_LDFLAGS=""
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
build

#x86_64
PLATFORM_VERSION=android-21
ARCH=x86_64
CPU=x86_64
PREFIX=$(pwd)/android_all/$CPU
TOOLCHAIN=$NDK_HOME/toolchains/x86_64-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/x86_64-linux-android-
ADDI_CFLAGS="-march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel"
ADDI_LDFLAGS=""
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
build

#mips
PLATFORM_VERSION=android-14
ARCH=mips
CPU=mips
PREFIX=$(pwd)/android_all/$CPU
TOOLCHAIN=$NDK_HOME/toolchains/mipsel-linux-android-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/mipsel-linux-android-
ADDI_CFLAGS=""
ADDI_LDFLAGS=""
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
#build

#mips64
PLATFORM_VERSION=android-21
ARCH=mips64
CPU=mips64
PREFIX=$(pwd)/android_all/$CPU
TOOLCHAIN=$NDK_HOME/toolchains/mips64el-linux-android-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/mips64el-linux-android-
ADDI_CFLAGS=""
ADDI_LDFLAGS=""
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
#build



