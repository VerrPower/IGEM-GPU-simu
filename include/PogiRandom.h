//
// Created by 31255 on 2025/6/26.
//

#ifndef POGIRANDOM_H
#define POGIRANDOM_H

#include "stdint.h"
#include "time.h"
#include "math.h"
#include "stdbool.h"

#define _2p32_ 4294967296.0
#define _inv_2p32_ 2.3283064365386963e-10
#define _32ln2_ 22.18070977791825  // 32 * ln2
#define PCG_MUL 6364136223846793005ULL
#define PCG_INC 1442695040888963407ULL

#define RN 3.6541528853610088 //            [128] 3.442619855899
#define VN 0.00492867323399 //                [128] 0.00991256303526217
#define NORM_R_INV 0.2736612373297583 //        [128] 0.290476451615 -> 1 / R
#define NORM_B0 6.070000482677376 //            [128] 6.442973870586131 -> 32 * ln2 / R

#define RE 7.69711747013104972 //
#define VE 0.0039496598225815571993 //

#define U32(generator) rnd_uint32(generator)

extern uint32_t kn[256];
extern double wn[256];
extern double fn[256];
extern uint32_t ke[256];
extern double we[256];
extern double fe[256];
extern bool ziggurat_initialized;

typedef struct PogiRandom{
    uint64_t state;
    double prob_t_2p32, _log1mpINV_;
    int geom_xmax;
    uint32_t tb_prob_t_2p32;
} PogiRandom;

void rnd_init_ziggurat();

void rnd_update_seed(PogiRandom *rnd);

void rnd_set_trunc_geom_param(PogiRandom *pcg, double geom_p, int geom_xmax);

uint32_t rnd_uint32(PogiRandom *rnd);

double rnd_uni01(PogiRandom *pcgRandom);

double rnd_norm(PogiRandom *rnd);

double rnd_expo(PogiRandom *pcg);

double rnd_mp1(PogiRandom *rnd);

int rnd_trunc_geom(PogiRandom *pcg);

int rnd_trunc_geom_arith(PogiRandom *pcg);

int rnd_trunc_geom_iter(PogiRandom *pcg);

void rnd_set_2pnt_param(PogiRandom *pcg, double tp_prob);

int rnd_2pnt(PogiRandom *pcg);

#endif


