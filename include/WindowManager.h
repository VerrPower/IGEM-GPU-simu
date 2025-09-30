//
// Created by 31255 on 2025/9/1.
//

#ifndef COLLISION_SIMULATION_CP1_WINDOWMANAGER_H
#define COLLISION_SIMULATION_CP1_WINDOWMANAGER_H

typedef struct GLFWwindow GLFWwindow;
extern GLFWwindow* global_window;
extern int winW, winH;

void window_manager_init();

void window_manager_cleanup();

int* window_get_winH_ptr();

int* window_get_winW_ptr();

#endif //COLLISION_SIMULATION_CP1_WINDOWMANAGER_H