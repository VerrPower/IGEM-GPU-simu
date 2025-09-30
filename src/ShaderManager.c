//
// Created by 31255 on 2025/6/28.
//

#include <stdio.h>
#include <string.h>
#include "ShaderManager.h"

inline char* safe_realloc(char* old, size_t new_size);
static char* load_shader_code_recursive(const char* path, int depth);
static void validate_shader(const GLuint *shader,const char *path);
static void validate_program(const GLuint *program);
static void load_compute_program(GLuint *compID, const char *path);
static void load_render_program(GLuint *renderID, GLint *loc_P, GLint *loc_V, const char *vert_path, const char *frag_path);


GLuint render_A, render_B, render_F;
GLuint comp_clr, comp_ins, comp_upd, comp_col;
GLint loc_V_A, loc_V_B, loc_V_F,
      loc_P_A, loc_P_B, loc_P_F;

static bool shader_initialized = false;

void shader_manager_init() {
    printf("@ ShaderManager: start to compile shaders...\n");
    load_render_program(&render_A, &loc_P_A, &loc_V_A, A_VERT_PATH, A_FRAG_PATH);
    load_render_program(&render_B, &loc_P_B, &loc_V_B, B_VERT_PATH, B_FRAG_PATH);
    load_render_program(&render_F, &loc_P_F, &loc_V_F, F_VERT_PATH, F_FRAG_PATH);

    load_compute_program(&comp_clr,COMP_PATH_CLEAR);
    load_compute_program(&comp_ins,COMP_PATH_INSERT);
    load_compute_program(&comp_col,COMP_PATH_COLLIDE);
    load_compute_program(&comp_upd,COMP_PATH_UPDATE);
    printf("@ ShaderManager: all shaders have been loaded\n");
    shader_initialized = true;
}


void shader_manager_use_render_and_set(GLuint program, GLint loc_P, GLint loc_V, mat4 P, mat4 V) {
    glUseProgram(program);
    glUniformMatrix4fv(loc_P, 1, GL_FALSE, (const float*)P);
    glUniformMatrix4fv(loc_V, 1, GL_FALSE, (const float*)V);
}

void shader_manager_use_program(GLuint compID) {
    glUseProgram(compID);
}

void shader_manager_cleanup() {
    glDeleteProgram(comp_clr);
    glDeleteProgram(comp_ins);
    glDeleteProgram(comp_col);
    glDeleteProgram(comp_upd);
    glDeleteProgram(render_A);
    glDeleteProgram(render_B);
    glDeleteProgram(render_F);
    printf("@ ShaderManager cleanup done\n");
}


inline char* safe_realloc(char* old, size_t new_size) {
    char* temp = realloc(old, new_size);
    if (!temp) {free(old);return NULL;}
    return temp;
}

static char* load_shader_code_recursive(const char* path, int depth) {
    if (depth > MAX_INCLUDE_DEPTH) {fprintf(stderr, "Include too deep: %s\n", path);return NULL;}
    FILE* f = fopen(path, "r");
    if (!f) {fprintf(stderr, "Failed to open shader: %s\n", path);return NULL;}
    char* output = NULL;
    size_t output_size = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "#include", 8) == 0) {
            char include_path[256];
            if (sscanf(line, "#include \"%255[^\"]\"", include_path) == 1) {
                char fullpath[512];
                char *dir = SHADER_ROOT;
                snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, include_path);
                char* included_code = load_shader_code_recursive(fullpath, depth + 1);
                if (included_code) {
                    size_t inc_len = strlen(included_code);
                    output = safe_realloc(output, output_size + inc_len + 1);
                    if (!output) {fclose(f);free(included_code);return NULL;}
                    memcpy(output + output_size, included_code, inc_len);
                    output_size += inc_len;
                    output[output_size] = '\0';
                    free(included_code);
                }
            }
        } else {
            size_t len = strlen(line);
            output = safe_realloc(output, output_size + len + 1);
            if (!output) {fclose(f);return NULL;}
            memcpy(output + output_size, line, len);
            output_size += len;
            output[output_size] = '\0';
        }
    }
    fclose(f);
    return output;
}

static void validate_shader(const GLuint *shader,const char *path) {
    int success;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    if (!success) {char log[512];glGetShaderInfoLog(*shader, 512, NULL, log);
        fprintf(stdout, "_____________shader compile error___________\n\t%s:\n%s\n",
            path, log);fflush(stdout);
    }
}

static void validate_program(const GLuint *program) {
    int success;
    glGetProgramiv(*program, GL_LINK_STATUS, &success);
    if (!success) {char log[512];glGetProgramInfoLog(*program, 512, NULL, log);
        fprintf(stderr,"____________\n program linkage error:\n%s\n",log);
    }
}

static void load_compute_program(GLuint *compID, const char *path) {
    char *comp_code = load_shader_code_recursive(path, 0);
    if (comp_code == NULL) {
        printf("failed to load shader from %s\n", path);
        return;
    }
    GLuint comp_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(comp_shader, 1, &comp_code, NULL);
    printf("\t compling shader: path = %s\n", path);
    glCompileShader(comp_shader);
    validate_shader(&comp_shader, (char *)path);
    *compID = glCreateProgram();
    glAttachShader(*compID, comp_shader);
    glLinkProgram(*compID);
    validate_program(compID);
    glDeleteShader(comp_shader);
    free(comp_code);
}

static void load_render_program(GLuint *renderID, GLint *loc_P, GLint *loc_V, const char *vert_path, const char *frag_path) {
    char *vert_code = load_shader_code_recursive(vert_path, 0);
    char *frag_code = load_shader_code_recursive(frag_path, 0);
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_code, NULL);
    glCompileShader(vert_shader);
    validate_shader(&vert_shader, vert_path);
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_code, NULL);
    glCompileShader(frag_shader);
    validate_shader(&frag_shader, frag_path);
    *renderID = glCreateProgram();
    glAttachShader(*renderID, vert_shader);
    glAttachShader(*renderID, frag_shader);
    glLinkProgram(*renderID);
    validate_program(renderID);
    *loc_P =  glGetUniformLocation(*renderID, "proj");
    *loc_V =  glGetUniformLocation(*renderID, "view");
    glDeleteShader(vert_shader); glDeleteShader(frag_shader);
    free(vert_code); free(frag_code);
}
