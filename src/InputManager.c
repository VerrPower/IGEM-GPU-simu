//
// Created by 31255 on 2025/9/2.
//
#include "InputManager.h"
#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

float aspect_ratio;
float ortho_H_ratio, ortho_W_ratio;
bool window_resized;
int mouse_left_pressed, mouse_right_pressed;
int key_w_pressed, key_s_pressed, key_a_pressed, key_d_pressed, key_tilde_pressed;
float current_mouse_x, current_mouse_y;
float last_mouse_x, last_mouse_y;
float mouse_dx, mouse_dy;
float accum_mouse_dx, accum_mouse_dy;
float accum_scroll_dx, accum_scroll_dy;

GLFWwindow *input_window;
int *input_winH, *input_winW;


void input_register(GLFWwindow* window, int *ptr_H, int *ptr_W) {
    input_window = window;
    input_winH = ptr_H;
    input_winW = ptr_W;
    ortho_H_ratio = 1.0f;
}

void input_poll_key_events() {
    key_w_pressed = glfwGetKey(input_window, GLFW_KEY_W);
    key_a_pressed = glfwGetKey(input_window, GLFW_KEY_A);
    key_s_pressed = glfwGetKey(input_window, GLFW_KEY_S);
    key_d_pressed = glfwGetKey(input_window, GLFW_KEY_D);
    key_tilde_pressed = glfwGetKey(input_window, GLFW_KEY_GRAVE_ACCENT);
}

void input_clear_state() {
    mouse_left_pressed = 0;
    mouse_right_pressed = 0;
    key_a_pressed = 0,   key_d_pressed = 0;
    key_w_pressed = 0;   key_s_pressed = 0;
    accum_mouse_dx = 0;  accum_mouse_dy = 0;
    accum_scroll_dx = 0; accum_scroll_dy = 0;
    mouse_dx = 0; mouse_dy = 0;
}

void input_clear_accum() {
    accum_mouse_dx = 0;
    accum_mouse_dy = 0;
    accum_scroll_dx = 0;
    accum_scroll_dy = 0;
}

void input_framebuffer_size_callback(GLFWwindow *winHandle, int new_winW, int new_winH) {
    glViewport(0, 0, new_winW, new_winH);
    ortho_H_ratio = (float)*input_winH / (float)new_winH;
    aspect_ratio = (float)new_winW / (float)new_winH;
    *input_winW = new_winW; *input_winH = new_winH;
    window_resized = true;
}

void input_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void input_scroll_callback(GLFWwindow *winHandle, double dx, double dy) {
    accum_scroll_dx += (float)dx;
    accum_scroll_dy += (float)dy;
}

void input_mouse_button_callback(GLFWwindow *winHandle,int _btn_, int _act_, int _mods_) {
    if (_btn_ != GLFW_MOUSE_BUTTON_LEFT) return;
    if (_act_ == GLFW_PRESS) {
        mouse_left_pressed = 1;
        last_mouse_x = current_mouse_x;
        last_mouse_y = current_mouse_y;
        accum_mouse_dx = 0.0f;
        accum_mouse_dy = 0.0f;
    } else {
        mouse_left_pressed = 0;
    }
}

void input_cursor_move_callback(GLFWwindow *winHandle,double xd, double yd) {
    //printf("@callback [cursor X/Y] [%.2f %.2f]\n", xd, yd);
    static bool first_move = true;
    float x = (float)xd; float y = (float)yd;
    if (first_move) {
        last_mouse_x = x;
        last_mouse_y = y;
        current_mouse_x = x;
        current_mouse_y = y;
        first_move = false;
    }
    mouse_dx = x - last_mouse_x;
    mouse_dy = y - last_mouse_y;
    last_mouse_x = x;
    last_mouse_y = y;
    current_mouse_x = x;
    current_mouse_y = y;
    if (mouse_left_pressed) {
        accum_mouse_dx += mouse_dx;
        accum_mouse_dy += mouse_dy;
    }

}