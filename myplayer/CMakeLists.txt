cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")
# cmakelists 设置c++11  因为在类中对非静态的初始化需要用到
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")

endif()
# cmakelists 设置c++11

include_directories(src/main/cpp/include)# 导入include，避免每次都写一长串

# -------------------- start  本地库 ------------------------

add_library( native-lib
             SHARED
             src/main/cpp/native-lib.cpp
             src/main/cpp/FFmpeg_Audio.cpp
             src/main/cpp/CallJava.cpp
             src/main/cpp/Audio.cpp
             src/main/cpp/Queue.cpp
             src/main/cpp/PlayStatus.cpp
        )

target_link_libraries( native-lib

                       avcodec-58
                       avdevice-58
                       avfilter-7
                       avformat-58
                       avutil-56
                       postproc-55
                       swresample-3
                       swscale-5

                        OpenSLES

                       log )

# -------------------- end  本地库 ------------------------



# ------------------------------------- start --------------------------------------

add_library( avcodec-58 SHARED IMPORTED)
set_target_properties( avcodec-58
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavcodec-58.so)


add_library( avdevice-58 SHARED IMPORTED)
set_target_properties( avdevice-58
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavdevice-58.so)


add_library( avfilter-7 SHARED IMPORTED)
set_target_properties( avfilter-7
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavfilter-7.so)


add_library( avformat-58 SHARED IMPORTED)
set_target_properties( avformat-58
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavformat-58.so)


add_library( avutil-56 SHARED IMPORTED)
set_target_properties( avutil-56
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavutil-56.so)

add_library( postproc-55 SHARED IMPORTED)
set_target_properties( postproc-55
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libpostproc-55.so)

add_library( swresample-3 SHARED IMPORTED)
set_target_properties( swresample-3
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswresample-3.so)


add_library( swscale-5 SHARED IMPORTED)
set_target_properties( swscale-5
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswscale-5.so)
# ------------------------------------- end --------------------------------------
