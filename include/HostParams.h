//
// Created by 31255 on 2025/9/3.
//

#ifndef COLLISION_SIMULATION_CP1_PARAMETERS_H
#define COLLISION_SIMULATION_CP1_PARAMETERS_H
#define Pi 3.14159265358979f

#define SAVE_SURVIVAL_SEQUENCE
#define VALIDATE_NORMALx
#define VALIDATE_EXPOx
#define VALIDATE_TWOPx
#define QUERY_GPU_INFOx
#define THREADPOOL_VERBOSE

// samples: run speed - NOR, run duration - * EXP, rot angle - NOR, rot duration - NOR,
// ============================== Simulation Settings =============================

// float: N N N E  int: 2P
#define RUN_SPEED_mean                           // -- device param.
#define RUN_SPEED_sigma                          // -- device param.
#define RUN_DURATION_mean                        // -- device param.
#define RUN_DURATION_sigma                       // -- device param.
#define ROTATION_DURATION_mean                   // -- device param.
#define ROTATION_ANGLE_mean                      // -- device param.
#define ROTATION_ANGLE_sigma                     // -- device param.

#define INIT_HEALTH_POINT   100
#define DAMAGE_PER_FRAME    0.03

#define AREA_UM2                                1e6                                                     // --sync src.
#define CAPSULE_PER_SQUARE_UM                   0.01                                                    // --sync src.
#define CIRCLE_PER_SQUARE_UM                    0.0013                                                  // --sync src.
#define CAPSULE_CENTER_DIST                     1.9f                                                    // --sync src.
#define CAPSULE_RADIUS                          0.5f                                                    // --sync src.
#define CIRCLE_RADIUS                           1.2f                                                    // --sync src.
#define MAX_CAPSULE_PER_CELL                    64                                                      // --sync src.
#define MAX_CIRCLE_PER_CELL                     32                                                      // --sync src.
#define MAX_COLLISIONS_CONCURRENT_PER_CAPSULE   64                                                      // --sync src.

#define NUM_CAPSULE                             ((int)(AREA_UM2 * CAPSULE_PER_SQUARE_UM))               // --sync src.
#define NUM_CIRCLE                              ((int)(AREA_UM2 * CIRCLE_PER_SQUARE_UM))                // --sync src.
#define BOUND_X                                 sqrtf(AREA_UM2)                                         // --sync src.
#define BOUND_Y                                 sqrtf(AREA_UM2)                                         // --sync src.
#define CELL_SIZE                               ((CAPSULE_CENTER_DIST + 2.0f * CAPSULE_RADIUS) * 3.0f)      // --sync src.
#define GRID_WIDTH                              ((int) ceilf(BOUND_X / CELL_SIZE))                      // --sync src.
#define GRID_HEIGHT                             ((int) ceilf(BOUND_Y / CELL_SIZE))                      // --sync src.
#define NUM_CELLS                               (GRID_WIDTH * GRID_HEIGHT)                              // --sync src.

#define VERTEX_COUNT                            32
#define PARTICLE_LINE_WIDTH                     0.008f

#define RENDER_RATE                             1320    // fps
#define LOGIC_RATE                              120   // ups
#define INFO_UPDATE_RATE                        10
#define RENDER_INTERVAL                         (1.0 / RENDER_RATE)
#define LOGIC_INTERVAL                          (1.0 / LOGIC_RATE)
#define INFO_UPDATE_INTERVAL                    (1.0 / INFO_UPDATE_RATE)

#define FRAME_RATE_SMOOTHING                    0.85

#define WG_SIZE                                 64                                                      // --sync src.
#define WORKLOAD_POW                            4                                                       // --sync src.
#define _workload_per_thread                    (1 << WORKLOAD_POW)
#define _num_of_clr_threads                     ((NUM_CELLS + _workload_per_thread - 1) / _workload_per_thread)
#define WG_NUM_CELL                             ((_num_of_clr_threads + WG_SIZE - 1) / WG_SIZE)
#define WG_NUM_CAP                              ((NUM_CAPSULE + WG_SIZE - 1) / WG_SIZE)

#define CAPSULE_MOTION_PORT                     1                                                       // --sync src.
#define CAPSULE_INDEX_TABLE_PORT                2                                                       // --sync src.
#define CAPSULE_COUNT_LIST_PORT                 3                                                       // --sync src.
#define CAPSULE_MODEL_PORT                      4                                                       // --sync src.
#define CAPSULE_INFO_PORT                       5                                                       // --sync src.
#define CIRCLE_STRUCT_PORT                      11                                                      // --sync src.
#define CIRCLE_INDEX_TABEL_PORT                 12                                                      // --sync src.
#define CIRCLE_COUNT_LIST_PORT                  13                                                      // --sync src.
#define CIRCLE_MODEL_PORT                       14                                                      // --sync src.
#define DEVICE_INT_BUF_PORT                     16                                                      // --sync src.
#define DEVICE_FLOAT_BUF_PORT                   17                                                      // --sync src.
#define DEBUG_PORT                              29                                                      // --sync src.
#define GLOBAL_COUNTER_PORT                     30                                                      // --sync src.
#define OUTPUT_PORT                             31                                                      // --sync src.

#define NORMAL_SEGMENT_START                    0
#define NORMAL_SEGMENT_END                      (NUM_CAPSULE * 3)
#define EXPO_SEGMENT_START                      (NUM_CAPSULE * 3)
#define EXPO_SEGMENT_END                        (NUM_CAPSULE * 4)
#define TWOP_SEGMENT_START                      0
#define TWOP_SEGMENT_END                        NUM_CAPSULE

#endif //COLLISION_SIMULATION_CP1_PARAMETERS_H