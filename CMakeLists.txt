cmake_minimum_required(VERSION 4.0)
project(checkpoint_RAT C)

set(CMAKE_C_STANDARD 11)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
endif()

# ==== 外部依赖路径（空格路径必须加引号） ====
set(my_include "${CMAKE_SOURCE_DIR}/include")
set(extern_include "D:/vcpkg/installed/x64-windows/include")
set(extern_lib "D:/vcpkg/installed/x64-windows/lib")

# ==== 公共源码 ====
set(SOURCE
        src/PogiRandom.c
        src/main.c
        src/ShaderManager.c
        src/WindowManager.c
        src/Camera.c
        src/Engine.c
        src/InputManager.c
        src/BufferManager.c
        src/ParallelRefiller.c
        include/HostParams.h
)

# ==== 静态库 libs ====
add_library(libs STATIC ${SOURCE})

target_include_directories(libs PUBLIC
        "${my_include}"
        "${extern_include}"
)

target_link_directories(libs PUBLIC
        "${extern_lib}"
)

# ==== 子目录 ====
add_subdirectory(src)
