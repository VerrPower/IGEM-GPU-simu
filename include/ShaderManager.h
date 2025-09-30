//
// Created by 31255 on 2025/6/28.
//

#ifndef SHADER_H
#define SHADER_H

#include <cglm/mat4.h>
#include <glad/glad.h>

#define MAX_INCLUDE_DEPTH 8
#define MAX_LINE_LENGTH 512

#define PATH2(a, b) a "/" b
#define PATH3(a, b, c) a "/" b "/" c
#define PATH4(a, b, c, d) a "/" b "/" c "/" d
#define PATH5(a, b, c, d, e) a "/" b "/" c "/" d "/" e
#define GET_MACRO(_1,_2,_3,_4,_5,NAME,...) NAME
#define PATH(...) GET_MACRO(__VA_ARGS__, PATH5, PATH4, PATH3, PATH2)(__VA_ARGS__)

#define SHADER_ROOT "D:/checkpoint - RAT/shaders"
    #define RENDER_DIR "render"
        #define RENDER_FILE_A "renderA"
        #define RENDER_FILE_B "renderB"
        #define RENDER_FILE_F "renderF"
    #define COMP_DIR "compute"
        #define COMP_FILE_CLEAR "clear_grid.comp"
        #define COMP_FILE_INSERT "insert_coords.comp"
        #define COMP_FILE_COLLIDE "detect_collision.comp"
        #define COMP_FILE_UPDATE "update_dynamics.comp"

#define A_VERT_PATH     PATH (SHADER_ROOT, RENDER_DIR, RENDER_FILE_A, "A.vert")
#define A_FRAG_PATH     PATH (SHADER_ROOT, RENDER_DIR, RENDER_FILE_A, "A.frag")
#define B_VERT_PATH     PATH (SHADER_ROOT, RENDER_DIR, RENDER_FILE_B, "B.vert")
#define B_FRAG_PATH     PATH (SHADER_ROOT, RENDER_DIR, RENDER_FILE_B, "B.frag")
#define F_VERT_PATH     PATH (SHADER_ROOT, RENDER_DIR, RENDER_FILE_F, "F.vert")
#define F_FRAG_PATH     PATH (SHADER_ROOT, RENDER_DIR, RENDER_FILE_F, "F.frag")

#define COMP_PATH_CLEAR       PATH (SHADER_ROOT, COMP_DIR, COMP_FILE_CLEAR)
#define COMP_PATH_INSERT      PATH (SHADER_ROOT, COMP_DIR, COMP_FILE_INSERT)
#define COMP_PATH_COLLIDE     PATH (SHADER_ROOT, COMP_DIR, COMP_FILE_COLLIDE)
#define COMP_PATH_UPDATE      PATH (SHADER_ROOT, COMP_DIR, COMP_FILE_UPDATE)


#define RENDER_PROGRAM_A 0
#define RENDER_PROGRAM_B 1
#define RENDER_PROGRAM_F 2
#define NUM_RENDER_PROGRAM 3

#define COMPUTE_PROGRAM_CLR 0
#define COMPUTE_PROGRAM_INS 1
#define COMPUTE_PROGRAM_COL 2
#define COMPUTE_PROGRAM_UPD 3
#define NUM_COMPUTE_PROGRAM 4

#define VIEW_MAT4 0
#define PROJ_MAT4 1
#define NUM_MAT4 2

typedef unsigned int SHADER_TYPE;

extern GLuint render_A, render_B, render_F;
extern GLuint comp_clr, comp_ins, comp_col, comp_upd;
extern GLint loc_V_A, loc_V_B, loc_V_F,
             loc_P_A, loc_P_B, loc_P_F;

void shader_manager_init();
void shader_manager_use_render_and_set(GLuint program, GLint loc_P, GLint loc_V, mat4 P, mat4 V);
void shader_manager_use_program(GLuint compID);
void shader_manager_cleanup();

#endif //SHADER_H
