# 测试可执行程序
add_executable(simu_exec_main
        main.c
)

# 链接到 libs（继承公共 include 路径）
target_link_libraries(simu_exec_main PRIVATE
        libs
        glfw3dll
        glad
        cglm
        opengl32
)

# 额外 include 路径（保证 config.h 及其依赖可找到）
target_include_directories(simu_exec_main PRIVATE
        "${extern_include}"
        "${my_include}"
        "../launcher"
)

# 链接库目录
target_link_directories(simu_exec_main PRIVATE
        "${extern_lib}"
)

# ==== 添加依赖：在编译 simu_exec_main 前同步 GLSL ====
