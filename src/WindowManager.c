//
// Created by 31255 on 2025/9/1.
//

#include "HostParams.h"
#include "InputManager.h"
#include "WindowManager.h"
#include <stdio.h>
#include <stdlib.h>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define WINDOW_WIDTH0 1920
#define WINDOW_HEIGHT0 1080
#define INITIAL_ORTHO_H 200
#define ZOOMING_FACTOR 0.81

GLFWwindow* global_window;
int winW, winH;

static void printComputeShaderLimits() {
    GLint maxSSBOBindings = 0;
    GLint64 maxSSBOSize = 0;
    GLint maxWorkGroupCount[3];
    GLint maxWorkGroupSize[3];
    GLint maxWorkGroupInvoc = 0;

    // 1. 最大 SSBO 绑定点数
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxSSBOBindings);

    // 2. 单个 SSBO 最大容量
    glGetInteger64v(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxSSBOSize);

    // 3. 最大工作组数 (x, y, z)
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkGroupCount[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxWorkGroupCount[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxWorkGroupCount[2]);

    // 4. 单工作组最大 invocation 数
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxWorkGroupInvoc);

    // 5. 每维度最大 local_size
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxWorkGroupSize[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxWorkGroupSize[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxWorkGroupSize[2]);

    printf("==== Compute Shader Limits ====\n");
    printf("Max SSBO bindings             = %d\n", maxSSBOBindings);
    printf("Max SSBO block size           = %lld bytes\n", (long long)maxSSBOSize);
    printf("Max work group count          = (%d, %d, %d)\n",
           maxWorkGroupCount[0], maxWorkGroupCount[1], maxWorkGroupCount[2]);
    printf("Max work group size           = (%d, %d, %d)\n",
           maxWorkGroupSize[0], maxWorkGroupSize[1], maxWorkGroupSize[2]);
    printf("Max invocations per workgroup = %d\n", maxWorkGroupInvoc);

    printf("================================\n");
}

void window_manager_init() {
    if (!glfwInit()) {fprintf(stderr,"GLFW init failed\n");return;}
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    global_window = glfwCreateWindow(WINDOW_WIDTH0,WINDOW_HEIGHT0, "hi! gl.", NULL, NULL);
    if (global_window == NULL) {
        fprintf(stderr,"fail to create global_window\n");
        glfwTerminate();
        return;
    }
    input_register(global_window, &winH, &winW);

    winW = WINDOW_WIDTH0;
    winH = WINDOW_HEIGHT0;
    aspect_ratio = (float)winW / (float)winH;

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int xpos = (mode->width - WINDOW_WIDTH0) / 2;
    int ypos = (mode->height - WINDOW_HEIGHT0) / 2;
    glfwSetWindowPos(global_window, xpos, ypos);

    glfwMakeContextCurrent(global_window);
    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(global_window, input_framebuffer_size_callback);
    glfwSetScrollCallback(global_window, input_scroll_callback);
    glfwSetMouseButtonCallback(global_window, input_mouse_button_callback);
    glfwSetCursorPosCallback(global_window, input_cursor_move_callback);
    glfwSetKeyCallback(global_window, input_key_callback);
    glfwSetCursorPos(global_window, (double)WINDOW_WIDTH0/2, (double)WINDOW_HEIGHT0/2);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        fprintf(stderr,"fail to init GLAD\n");
    #ifdef QUERY_GPU_INFO
        printComputeShaderLimits();
    #endif
    printf("@ WindowManager init done\n");fflush(stdout);
}

void window_manager_cleanup() {
    glfwDestroyWindow(global_window);
    printf("@ WindowManager nullified\n");fflush(stdout);
}

int* window_get_winH_ptr() {
    return &winH;
}

int* window_get_winW_ptr() {
    return &winW;
}
