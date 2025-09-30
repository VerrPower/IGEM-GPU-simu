//
// Created by 31255 on 2025/9/2.
//
#include "HostParams.h"
#include "PogiRandom.h"
#include <stdlib.h>
#include <math.h>
#include <cglm/cglm.h>
#include "ParallelRefiller.h"
#include "BufferManager.h"

int    vao_cnt, vbo_cnt, ssbo_cnt, ubo_cnt;
GLuint capsule_vao, circle_vao, bound_vao;
GLuint capsule_vbo, circle_vbo, bound_vbo;

GLuint capsule_struct_buffer, capsule_model_count,  capsule_count_list_buffer,  capsule_index_table_buffer, capsule_info_buffer;
GLuint circle_struct_buffer,  circle_model_buffer,  circle_count_list_buffer,   circle_index_table_buffer;

GLuint device_int_buffer, device_float_buffer;
GLuint output_buffer, debug_buffer, global_counter_buffer;
uint64_t total_mem_bytesize;
int   *host_int_buffer;
float *host_float_buffer;

static void buffer_init_threadpool();
static void buffer_init_shader_source_buffer();
static void buffer_init_vertex_buffer();
static void init_vao_vbo(GLuint *vao, GLuint *vbo, GLsizeiptr vbo_size, const float *data);
static void alloc_ssbo(GLuint *buffer, BINDING_PORT port, GLsizeiptr buffer_size, GLenum usage);
static void alloc_ubo(GLuint *buffer, BINDING_PORT port, GLsizeiptr buffer_size, GLenum usage);
static void reset_global_counter();


void buffer_manager_cleanup() {
    glDeleteBuffers(1, &debug_buffer);
    glDeleteBuffers(1, &global_counter_buffer);

    glDeleteBuffers(1, &capsule_struct_buffer);
    glDeleteBuffers(1, &capsule_model_count);
    glDeleteBuffers(1, &capsule_count_list_buffer);
    glDeleteBuffers(1, &capsule_index_table_buffer);
    glDeleteBuffers(1, &capsule_info_buffer);

    glDeleteBuffers(1, &circle_struct_buffer);
    glDeleteBuffers(1, &circle_model_buffer);
    glDeleteBuffers(1, &circle_count_list_buffer);
    glDeleteBuffers(1, &circle_index_table_buffer);

    glDeleteBuffers(1, &device_int_buffer);
    glDeleteBuffers(1, &device_float_buffer);
    glDeleteBuffers(1, &output_buffer);

    glDeleteVertexArrays(1, &capsule_vao);
    glDeleteVertexArrays(1, &circle_vao);
    glDeleteVertexArrays(1, &bound_vao);

    glDeleteBuffers(1, &capsule_vbo);
    glDeleteBuffers(1, &circle_vbo);
    glDeleteBuffers(1, &bound_vbo);

    free(host_float_buffer);
    free(host_int_buffer);
    parafill_cleanup();
    printf("@ BufferManager cleanup done\n");
}

static bool buffer_initialized = false;

void buffer_manager_init() {
    vao_cnt = 0;
    vbo_cnt = 0;
    ssbo_cnt = 0;
    total_mem_bytesize = 0;
    buffer_init_threadpool();
    buffer_init_shader_source_buffer();
    buffer_init_vertex_buffer();
    buffer_initialized = true;
    printf("@ BufferManager init done\n");
}

void buffer_manager_refill_host_buffer() {

}

static void buffer_init_threadpool() {
    host_float_buffer = malloc(sizeof(float) * NUM_CAPSULE * 4);
    host_int_buffer   = malloc(sizeof(int)   * NUM_CAPSULE);
    register_host_buffer(host_int_buffer, host_float_buffer);
    parafill_set_fill_segment(
        NORMAL_SEGMENT_START, NORMAL_SEGMENT_END,
        EXPO_SEGMENT_START, EXPO_SEGMENT_END,
        TWOP_SEGMENT_START, TWOP_SEGMENT_END
        );
    parafill_init_threads();
}

static void buffer_init_shader_source_buffer() {
    alloc_ssbo(&global_counter_buffer,       GLOBAL_COUNTER_PORT,  sizeof(GLuint),                               GL_DYNAMIC_DRAW);
    alloc_ssbo(&debug_buffer,                DEBUG_PORT,           sizeof(int),                                  GL_DYNAMIC_DRAW);

    alloc_ssbo(&capsule_struct_buffer,       CAPSULE_MOTION_PORT,  NUM_CAPSULE * sizeof(Capsule),                GL_DYNAMIC_COPY);
    alloc_ssbo(&capsule_count_list_buffer,   CAPSULE_COUNT_LIST_PORT,  NUM_CELLS * sizeof(GLuint),                   GL_DYNAMIC_DRAW);
    alloc_ssbo(&capsule_index_table_buffer,  CAPSULE_INDEX_TABLE_PORT,  NUM_CELLS * MAX_CAPSULE_PER_CELL * sizeof(GLuint),      GL_DYNAMIC_DRAW);
    alloc_ssbo(&capsule_model_count,         CAPSULE_MODEL_PORT,   NUM_CAPSULE * sizeof(mat4),                   GL_DYNAMIC_DRAW);
    alloc_ssbo(&capsule_info_buffer,         CAPSULE_INFO_PORT,    NUM_CAPSULE * sizeof(CapsuleInfoBuf),         GL_DYNAMIC_DRAW);

    alloc_ssbo(&circle_struct_buffer,        CIRCLE_STRUCT_PORT,   NUM_CIRCLE * sizeof(Circle),                  GL_DYNAMIC_DRAW);
    alloc_ssbo(&circle_count_list_buffer,    CIRCLE_COUNT_LIST_PORT,   NUM_CELLS * sizeof(GLuint),                   GL_STATIC_DRAW);
    alloc_ssbo(&circle_index_table_buffer,   CIRCLE_INDEX_TABEL_PORT,   NUM_CELLS * MAX_CIRCLE_PER_CELL * sizeof(GLuint), GL_STATIC_DRAW);
    alloc_ssbo(&circle_model_buffer,         CIRCLE_MODEL_PORT,    NUM_CIRCLE * sizeof(mat4),                    GL_STATIC_DRAW);

    alloc_ssbo(&device_float_buffer,         DEVICE_FLOAT_BUF_PORT,NUM_CAPSULE * 4 * sizeof(float),              GL_DYNAMIC_DRAW);
    alloc_ssbo(&device_int_buffer,           DEVICE_INT_BUF_PORT,  NUM_CAPSULE * sizeof(int),                    GL_DYNAMIC_DRAW);
    alloc_ssbo(&output_buffer,               OUTPUT_PORT,          NUM_CAPSULE * sizeof(float),                  GL_DYNAMIC_READ);

    reset_global_counter();

    // ------------------ generate C.auris -------------------
    float L20 = (4.0f * CIRCLE_RADIUS * CIRCLE_RADIUS);
    float L21 = (4.0f * CAPSULE_RADIUS * CAPSULE_RADIUS);
    float L22 = ((CAPSULE_RADIUS + CIRCLE_RADIUS) * (CAPSULE_RADIUS + CIRCLE_RADIUS));

    Circle  *circle_motion = map_ssbo_write( circle_struct_buffer,NUM_CIRCLE * sizeof(Circle));
    GLuint  *circle_cnt    = map_ssbo_write( circle_count_list_buffer,NUM_CELLS * sizeof(GLuint));
    GLuint  *circle_index  = map_ssbo_write( circle_index_table_buffer,NUM_CELLS * MAX_CIRCLE_PER_CELL * sizeof(GLuint));
    mat4    *circle_model  = map_ssbo_write( circle_model_buffer,NUM_CIRCLE * sizeof(mat4));

    PogiRandom pogiRnd;
    rnd_update_seed(&pogiRnd);
    for (GLuint cid = 0; cid < NUM_CIRCLE; ++cid) {
        Circle *C = &circle_motion[cid];
        CAURIS_GENERATION:
        vec2 pos; pos[0] = rnd_uni01(&pogiRnd) * BOUND_X; pos[1] = rnd_uni01(&pogiRnd) * BOUND_Y;
        if (pos[0] < CIRCLE_RADIUS || pos[1] < CIRCLE_RADIUS
            || pos[0] > BOUND_X-CIRCLE_RADIUS || pos[1] > BOUND_Y-CIRCLE_RADIUS) goto CAURIS_GENERATION;
        int Cx = (int)(pos[0] / CELL_SIZE);
        int Cy = (int)(pos[1] / CELL_SIZE);
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                int nCx = Cx + dx, nCy = Cy + dy;
                if (nCx < 0 || nCy < 0 || nCx >= GRID_WIDTH || nCy >= GRID_HEIGHT) continue;
                int nCellId = nCx + nCy * GRID_WIDTH;
                GLuint count = circle_cnt[nCellId];
                for (GLuint i = 0; i < count; ++i) {
                    GLuint oid = circle_index[nCellId * MAX_CIRCLE_PER_CELL + i];
                    Circle *Of = &circle_motion[oid];
                    vec2 pos2, dist;
                    pos2[0] = Of->pos[0]; pos2[1] = Of->pos[1];
                    glm_vec2_sub(pos, pos2, dist);
                    float MD2 = glm_vec2_dot(dist, dist);
                    if (MD2 < L20) goto CAURIS_GENERATION;
                }
            }
        }
        C->pos[0] = pos[0]; C->pos[1] = pos[1];
        C->health = 3.0f;
        glm_mat4_identity(circle_model[cid]);
        circle_model[cid][3][0] = pos[0]; circle_model[cid][3][1] = pos[1];
        GLuint cell_id = Cx + Cy * GRID_WIDTH;
        GLuint idx = cell_id * MAX_CIRCLE_PER_CELL + circle_cnt[cell_id];
        circle_index[idx] = cid;
        circle_cnt[cell_id] += 1;
    }
    unmap_ssbo(circle_model_buffer);



    // ------------------ generate E.coli --------------------
    Capsule *capsule_motion = map_ssbo_write(capsule_struct_buffer,     NUM_CAPSULE * sizeof(Capsule));
    uint64_t  *capsule_cnt    = map_ssbo_write(capsule_count_list_buffer, NUM_CELLS * sizeof(GLuint));
    GLuint  *capsule_index  = map_ssbo_write(capsule_index_table_buffer,NUM_CELLS * MAX_CAPSULE_PER_CELL * sizeof(GLuint));

    float a, c, theta;
    a = c = CAPSULE_CENTER_DIST * CAPSULE_CENTER_DIST;
    for (int eid = 0; eid < NUM_CAPSULE; ++eid) {
        CAPSULE_GENERATION:
        vec2 pos, dir, tmp, P1, u, v, w;
        pos[0] = rnd_uni01(&pogiRnd) * BOUND_X; pos[1] = rnd_uni01(&pogiRnd) * BOUND_Y;
        int Cx = (int)(pos[0] / CELL_SIZE), Cy = (int)(pos[1] / CELL_SIZE);
        theta = rnd_uni01(&pogiRnd) * 2.0f * M_PI;
        dir[0] = cosf(theta); dir[1] = sinf(theta);
        glm_vec2_scale(dir, CAPSULE_CENTER_DIST/2, tmp);
        glm_vec2_sub(pos, tmp, P1);
        float b,d,e,D,s,t;
        glm_vec2_scale(dir, CAPSULE_CENTER_DIST, u);
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                int nCx = Cx + dx, nCy = Cy + dy;
                if (nCx<0 || nCy<0 || nCx>=GRID_WIDTH || nCy>=GRID_HEIGHT) continue;
                GLuint nCID = (GLuint)nCx + (GLuint)nCy * GRID_WIDTH;
                GLuint count_C = circle_cnt[nCID];
                for (GLuint i = 0; i < count_C; ++i) {
                    GLuint oid = circle_index[nCID * MAX_CIRCLE_PER_CELL + i];
                    Circle *Of = &circle_motion[oid];
                    vec2 Q, P0, MD;
                    Q[0] = Of->pos[0]; Q[1] = Of->pos[1];
                    glm_vec2_sub(P1, Q, v);
                    b = glm_vec2_dot(u, v);
                    t = glm_clamp(-b/a, 0, 1);
                    glm_vec2_scale(u, t, tmp);
                    glm_vec2_add(P1, tmp, P0);
                    glm_vec2_sub(P0, Q, MD);
                    float MD2 = glm_vec2_dot(MD, MD);
                    if (MD2 < L22) goto CAPSULE_GENERATION;
                }
                GLuint count_E = capsule_cnt[nCID];
                for (int i = 0; i < count_E; ++i) {
                    GLuint oid = capsule_index[nCID * MAX_CAPSULE_PER_CELL + i];
                    if (oid>=eid) printf("err\n");
                    Capsule *cap2 = &capsule_motion[oid];
                    vec2 pos2, dir2, Q1,P0, Q0, MD;
                    pos2[0] = cap2->pos[0]; pos2[1] = cap2->pos[1];
                    float theta2 = cap2->theta;
                    dir2[0] = cosf(theta2); dir2[1] = sinf(theta2);
                    glm_vec2_scale(dir2, CAPSULE_CENTER_DIST/2, tmp);
                    glm_vec2_sub(pos2, tmp, Q1);
                    glm_vec2_scale(dir2, CAPSULE_CENTER_DIST, v);
                    glm_vec2_sub(P1, Q1, w);
                    b = glm_vec2_dot(u, v);
                    D = a * c - b*b;
                    d = glm_vec2_dot(v, w);
                    e = glm_vec2_dot(u, w);
                    if (-1e-8 < D && D < 1e-8) s = 0;
                    else s = glm_clamp((b*d - c*e)/D, 0, 1);
                    t = (b*s + d)/c;
                    if (t>1 || t<0) {
                        t = glm_clamp(t, 0, 1);
                        s = glm_clamp((b*t - e)/a, 0, 1);
                    }
                    glm_vec2_scale(u, s, tmp);
                    glm_vec2_add(P1, tmp, P0);
                    glm_vec2_scale(v, t, tmp);
                    glm_vec2_add(Q1, tmp, Q0);
                    glm_vec2_sub(P0, Q0, MD);
                    float MD2 = glm_vec2_dot(MD, MD);
                    if (MD2 < L21) goto CAPSULE_GENERATION;
                }
            }
        }
        GLuint cell_id = (GLuint)Cx + (GLuint)Cy * GRID_WIDTH;
        GLuint local_slot = capsule_cnt[cell_id];
        GLuint global_slot = cell_id * MAX_CAPSULE_PER_CELL + local_slot;
        capsule_index[global_slot] = eid;
        capsule_cnt[cell_id] = local_slot + 1;

        Capsule *P = &capsule_motion[eid];
        P->pos[0] = pos[0]; P->pos[1] = pos[1];
        P->theta = theta;

        vec2 v0; v0[0] = rnd_mp1(&pogiRnd); v0[1] = rnd_mp1(&pogiRnd);
        glm_vec2_normalize(v0);
        glm_vec2_scale(v0, 3.0f, v0);
        P->prev_pos[0] = pos[0] - v0[0] * LOGIC_INTERVAL;
        P->prev_pos[1] = pos[1] - v0[1] * LOGIC_INTERVAL;
        P->acc[0] = 0.0f; P->acc[1] = 0.0f;
        float w0 = rnd_mp1(&pogiRnd);
        P->prev_theta = theta - w0 * LOGIC_INTERVAL;
        P->angular_acc = 0.0f;

        P->overlapped = 0;
        P->motion_state = 0;
        P->cooldown = 640;
    }
    unmap_ssbo(capsule_struct_buffer);
    unmap_ssbo(capsule_count_list_buffer);
    unmap_ssbo(capsule_index_table_buffer);
    unmap_ssbo(circle_struct_buffer);
    unmap_ssbo(circle_count_list_buffer);
    unmap_ssbo(circle_index_table_buffer);
}

static void buffer_init_vertex_buffer() {
    float circle_data[(VERTEX_COUNT + 2) * 3];
    GLsizeiptr circle_sz = (VERTEX_COUNT + 2) * 3 * sizeof(float);
    for (int i = 0; i < 3; ++i) circle_data[i] = 0.0f;
    for (int  i= 1; i < VERTEX_COUNT + 2; i++) {
        float angle = GLM_PI * 2.0f / VERTEX_COUNT * i;
        circle_data[3*i+0] = cosf(angle) * CIRCLE_RADIUS;
        circle_data[3*i+1] = sinf(angle) * CIRCLE_RADIUS;
        circle_data[3*i+2] = 0;
    }
    float capsule_data[VERTEX_COUNT * 3];
    GLsizeiptr capsule_sz = VERTEX_COUNT * 3 * sizeof(float);
    for (int i = 0; i < VERTEX_COUNT; ++i) {
        float angle = GLM_PI * 2.0f/(float)VERTEX_COUNT * i + glm_rad(90);
        float sign = i >= VERTEX_COUNT/2 ? 1 : -1;
        capsule_data[3*i+0] = cosf(angle) * CAPSULE_RADIUS + sign * CAPSULE_CENTER_DIST/2;
        capsule_data[3*i+1] = sinf(angle) * CAPSULE_RADIUS;
        capsule_data[3*i+2] = 0;
    }
    float bound_data[12]={
        0,0, 0,
        BOUND_X, 0, 0,
        BOUND_X, BOUND_Y, 0,
        0, BOUND_Y, 0,
    };
    init_vao_vbo(&circle_vao,  &circle_vbo, circle_sz, circle_data);
    init_vao_vbo(&capsule_vao, &capsule_vbo, capsule_sz, capsule_data);
    init_vao_vbo(&bound_vao,   &bound_vbo, 48, bound_data);
}

static void init_vao_vbo(GLuint *vao, GLuint *vbo, GLsizeiptr vbo_size, const float *data) {
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_size, data, GL_DYNAMIC_DRAW);
    glBindVertexArray(*vao);glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

static void alloc_ssbo(GLuint *buffer, BINDING_PORT port, GLsizeiptr buffer_size, GLenum usage) {
    glGenBuffers(1, buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, NULL, usage);
    total_mem_bytesize += buffer_size;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, port, *buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    ssbo_cnt ++;
}

static void alloc_ubo(GLuint *buffer, BINDING_PORT port, GLsizeiptr buffer_size, GLenum usage) {
    glGenBuffers(1, buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, *buffer);
    glBufferData(GL_UNIFORM_BUFFER, buffer_size, NULL, usage);
    total_mem_bytesize += buffer_size;
    glBindBufferBase(GL_UNIFORM_BUFFER, port, *buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    ubo_cnt ++;
}

static void reset_global_counter() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, global_counter_buffer);
    GLuint zero = 0;
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &zero);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void fill_ssbo(GLuint ssbo, const void *data, GLsizeiptr size) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void fill_ubo(GLuint ubo, const void *data, GLsizeiptr size) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void * map_ssbo_write(GLuint ssbo, GLsizeiptr length) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    return glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        0, length,
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT
        );
}

void * map_ssbo_read(GLuint ssbo, GLsizeiptr length) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    return glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        0, length,
        GL_MAP_READ_BIT
        );
}

void unmap_ssbo(GLuint ssbo) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}