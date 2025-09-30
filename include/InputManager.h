//
// Created by 31255 on 2025/9/2.
//

#ifndef COLLISION_SIMULATION_CP1_INPUTMANAGER_H
#define COLLISION_SIMULATION_CP1_INPUTMANAGER_H

#include "stdbool.h"

extern float aspect_ratio;
extern float ortho_H_ratio;
extern bool window_resized;
extern int mouse_left_pressed, mouse_right_pressed;
extern int key_w_pressed, key_s_pressed, key_a_pressed, key_d_pressed, key_tilde_pressed;
extern float current_mouse_x, current_mouse_y;
extern float last_mouse_x, last_mouse_y;
extern float mouse_dx, mouse_dy;
extern float accum_mouse_dx, accum_mouse_dy;
extern float accum_scroll_dx, accum_scroll_dy;

typedef struct GLFWwindow GLFWwindow;

void input_register(GLFWwindow* window, int *winH, int *winW);

void input_clear_state();

void input_poll_key_events();

void input_framebuffer_size_callback(GLFWwindow *winHandle, int new_winW, int new_winH);

void input_scroll_callback(GLFWwindow *winHandle, double dx, double dy);

void input_mouse_button_callback(GLFWwindow *winHandle, int _btn_, int _act_, int _mods_);

void input_cursor_move_callback(GLFWwindow *winHandle,double xd, double yd);

void input_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void input_clear_accum();

#endif //COLLISION_SIMULATION_CP1_INPUTMANAGER_H