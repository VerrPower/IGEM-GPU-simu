//
// Created by 31255 on 2025/9/2.
//

#include "Camera.h"
#include "InputManager.h"
#include "HostParams.h"

// NOTE : LEFT-HAND axis is employed by default

#define fill_vec3(v, x, y, z) v[0]=x; v[1]=y; v[2]= z;
#define calc_dir(dir, θ, ϕ) dir[0]=cosf(ϕ)*cosf(θ); dir[1]=sinf(ϕ); dir[2]=cosf(ϕ)*sinf(θ);

#define ORTHO_Z_NEAR 0.01f
#define ORTHO_Z_FAR 100.0f
#define ORTHO_SCROLL_ROOM_FACTOR 0.90f
#define ORTHO_HEIGHT_SCALE_FACTOR 0.9f

static vec3 world_up;
vec3 cameraPos, cameraDir;
float cameraYaw, cameraPitch;
float ortho_l, ortho_r, ortho_b, ortho_t;
mat4 projectionMatrix, viewMatrix;

int *camera_winH, *camera_winW;

void camera_register(int *winH, int *winW) {
    camera_winH = winH;
    camera_winW = winW;
}

void camera_init_matrix() {
    fill_vec3(world_up, 0.0f, 1.0f, 0.0f);
    fill_vec3(cameraPos, 0.0f, 0.0f, 10.0f);
    fill_vec3(cameraDir, 0.0f, 0.0f, -1.0f);
    glm_look(cameraPos, cameraDir, world_up, viewMatrix);
    float ortho_centerX = (0 + BOUND_X) / 2.0f;
    float ortho_centerY = (0 + BOUND_Y) / 2.0f;
    float ortho_H = BOUND_X * 1.3f;
    float ortho_W = ortho_H * aspect_ratio;
    ortho_l = ortho_centerX - ortho_W / 2.0f;
    ortho_r = ortho_centerX + ortho_W / 2.0f;
    ortho_b = ortho_centerY - ortho_H / 2.0f;
    ortho_t = ortho_centerY + ortho_H / 2.0f;
    glm_ortho(
        ortho_l, ortho_r, ortho_b, ortho_t,
        ORTHO_Z_NEAR, ORTHO_Z_FAR, projectionMatrix
        );
}

void camera_update_ortho_projection_matrix() {
    if (window_resized) {
        float ortho_centerX = (ortho_r + ortho_l) / 2.0f;
        float ortho_centerY = (ortho_t + ortho_b) / 2.0f;
        ortho_b = ortho_centerY + (ortho_b - ortho_centerY) * ortho_H_ratio * ORTHO_HEIGHT_SCALE_FACTOR;
        ortho_t = ortho_centerY + (ortho_t - ortho_centerY) * ortho_H_ratio * ORTHO_HEIGHT_SCALE_FACTOR;
        float ortho_W = (ortho_t - ortho_b) * aspect_ratio;
        ortho_r = ortho_centerX - ortho_W / 2.0f;
        ortho_l = ortho_centerX + ortho_W / 2.0f;
        window_resized = false;
    }
    float scale_y = (ortho_t - ortho_b) / (float)(*camera_winH);
    float scale_x = (ortho_r - ortho_l) / (float)(*camera_winW);
    float world_dX = accum_mouse_dx * scale_x;
    float world_dY = accum_mouse_dy * scale_y;
    ortho_l -= world_dX;
    ortho_r -= world_dX;
    ortho_b += world_dY;
    ortho_t += world_dY;
    if (accum_scroll_dy != 0.0f) {
        float world_cursorX = ortho_l + current_mouse_x / (float)(*camera_winW) * (ortho_r - ortho_l);
        float world_cursorY = ortho_t - current_mouse_y / (float)(*camera_winH) * (ortho_t - ortho_b);
        float zoom = powf(ORTHO_SCROLL_ROOM_FACTOR, accum_scroll_dy);
        ortho_l = world_cursorX + (ortho_l - world_cursorX) * zoom;
        ortho_r = world_cursorX + (ortho_r - world_cursorX) * zoom;
        ortho_b = world_cursorY + (ortho_b - world_cursorY) * zoom;
        ortho_t = world_cursorY + (ortho_t - world_cursorY) * zoom;
    }
    glm_ortho(
            ortho_l,ortho_r,ortho_b,ortho_t,
            ORTHO_Z_NEAR,ORTHO_Z_FAR, projectionMatrix
            );
    /*if (accum_scroll_dy!=0 || (accum_mouse_dx!=0 && accum_mouse_dy!=0))
        printf("[accum_mouse_offset X/Y | accum_scroll_offset X/Y] [%.2f %.2f | %.2f %.2f]\n",
         accum_mouse_dx, accum_mouse_dy, accum_scroll_dx, accum_scroll_dy
        );*/

    input_clear_accum();
}

inline void update_forward_and_right(float θ, float *F, float *R) {
    float s = sinf(θ), c = cosf(θ);
    fill_vec3(F, c, 0.0f, s);
    fill_vec3(R, s, 0.0f, -c);
}
