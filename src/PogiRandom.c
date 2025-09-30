//
// Created by 31255 on 2025/6/26.
//

#include <stdio.h>
#include "PogiRandom.h"
#include <stdlib.h>

uint32_t kn[256];
double wn[256];
double fn[256];
uint32_t ke[256];
double we[256];
double fe[256];
bool ziggurat_initialized = false;



inline double f_expo(double x);
inline double inv_f_expo(double x);
inline double f_norm(double x);
inline double inv_f_norm(double x);

void rnd_init_ziggurat() {
    double xn = RN;
    double xe = RE;
    for (int i = 256 - 1; i >= 1 ;--i) {
        double y_norm = inv_f_norm(VN / xn + f_norm(xn));
        kn[i] = (uint32_t)(_2p32_ * y_norm / xn);
        wn[i] = xn * _inv_2p32_;
        fn[i] = f_norm(xn);
        xn = y_norm;
        double y_expo = inv_f_expo(VE / xe + f_expo(xe));
        ke[i] = (uint32_t)(_2p32_ * y_expo / xe);
        we[i] = xe * _inv_2p32_;
        fe[i] = f_expo(xe);
        xe = y_expo;
    }
    kn[0] = (uint32_t)(_2p32_ * (RN * f_norm(RN)) / VN);
    wn[0] = VN / (f_norm(RN) * _2p32_);
    fn[0] = 1;
    ke[0] = (uint32_t)(_2p32_ * (RE * f_expo(RE)) / VE);
    we[0] = VE / (f_expo(RE) * _2p32_);
    fe[0] = 1;
    ziggurat_initialized = true;
}

void rnd_update_seed(PogiRandom *rnd) {
    uint64_t u64 = ((uint64_t) time(NULL) << 32) ^ (uint64_t) clock();
    uint64_t u64_p = (uintptr_t)&(rnd->state);
    u64_p &= 1023; u64_p += PCG_INC; u64_p *= PCG_MUL;
    u64_p ^= u64_p << 13u; u64_p ^= u64_p >> 7u; u64 ^= u64_p << 17u;
    rnd->state = u64;
    //printf("@ PogiRandom - seed is set to: %llu\n",u64_p);
}

double rnd_norm(PogiRandom *rnd){
    if (!ziggurat_initialized) {
        printf("forced ziggurat init\n");
        rnd_init_ziggurat();
    }
    while (true) {
        uint32_t j = U32(rnd), i = j & 255, sign = j ^ j<<13;
        double x = j * wn[i];
        sign ^= sign>>18; sign >>= 27; sign &= 1;
        if(j < kn[i]) return sign? x : -x;
        if(i > 0){
            if((f_norm(x)-fn[i]) * _2p32_ >(fn[i-1]-fn[i])* (double)U32(rnd))
                return sign? x : -x;
        }else{
            double y;
            do{x = NORM_B0 - NORM_R_INV * log(U32(rnd));
                y = _32ln2_ - log(U32(rnd));
            }while(y+y < x*x);
            return sign? x+RN : -x-RN;
        }
    }
}

double rnd_expo(PogiRandom *pcg){
    if (!ziggurat_initialized) {
        printf("forced ziggurat init\n");
        rnd_init_ziggurat();
    }
    while (true) {
        uint32_t j = U32(pcg);
        uint32_t i = j & 255;
        double x = j * we[i];
        if(j < ke[i])return x;
        if(i > 0){
            if((f_expo(x)-fe[i]) * _2p32_ >(fe[i-1]-fe[i])* (double)U32(pcg)) return x;
        }
        else return RE - log(rnd_uni01(pcg));
    }
}

uint32_t rnd_uint32(PogiRandom *rnd){
    uint64_t old_st = rnd->state;
    rnd->state = old_st * PCG_MUL + PCG_INC;
    uint32_t xorSft = (uint32_t)(((old_st >> 18u) ^ old_st) >> 27u);
    uint32_t rot = old_st >> 59u;
    uint32_t xorSftRot = (xorSft >> rot) | (xorSft << (-rot & 31));
    return xorSftRot;
}

double rnd_uni01(PogiRandom *pcgRandom){
    return (double)(U32(pcgRandom)) * _inv_2p32_;
}

double rnd_mp1(PogiRandom *rnd){
    uint64_t cur_state = rnd->state;
    rnd->state = cur_state * PCG_MUL + PCG_INC;
    uint32_t xorSft = (uint32_t)((cur_state >> 18u ^ cur_state) >> 27u);
    uint32_t rot = cur_state >> 59u;
    uint32_t xorSftRot = xorSft >> rot | xorSft << (-rot & 31);
    uint32_t sign = cur_state ^ cur_state<<13;
    sign ^= sign>>17; sign^= sign<<19; sign &= 0x80000000;
    return sign? (double)xorSftRot * _inv_2p32_ : -(double)xorSftRot * _inv_2p32_;
}


void rnd_set_2pnt_param(PogiRandom *pcg, double tp_prob) {
    pcg->tb_prob_t_2p32 = floor(tp_prob * _2p32_);
}

int rnd_2pnt(PogiRandom *pcg) {
    return rnd_uint32(pcg) > pcg->tb_prob_t_2p32 ? 0 : 1;
}

int rnd_trunc_geom_arith(PogiRandom *pcg) {
    int xmax = pcg->geom_xmax;
    SCENARIO_1:
    int x = (int) floor(log(rnd_uni01(pcg)) * pcg->_log1mpINV_);
    if (x > xmax) goto SCENARIO_1;
    return x;
}

int rnd_trunc_geom_iter(PogiRandom *pcg) {
    int xmax = pcg->geom_xmax;
    double l = pcg->prob_t_2p32;;
    SCENARIO_2:
    int k = 0;
    while (rnd_uint32(pcg) > l) k++;
    if (k > xmax) goto SCENARIO_2;
    return k;
}

int rnd_trunc_geom(PogiRandom *pcg){
    int xmax = pcg->geom_xmax;
    if (pcg->prob_t_2p32 < 0.25 * _2p32_) {
        SCENARIO_1:
        int x = (int)floor(log(rnd_uni01(pcg)) * pcg->_log1mpINV_);
        if (x > xmax) goto SCENARIO_1;
        return x;
    } else {
        SCENARIO_2:
        int k = 0;
        double geom_p = pcg->prob_t_2p32;
        while (rnd_uint32(pcg) > geom_p) k++;
        if (k > xmax) goto SCENARIO_2;
        return k;
    }
}


void rnd_set_trunc_geom_param(PogiRandom *pcg, double geom_p, int geom_xmax) {
    pcg->prob_t_2p32 = geom_p * _2p32_;
    pcg->geom_xmax = geom_xmax;
    pcg->_log1mpINV_ = 1.0 / log(1.0 - geom_p);
}

double f_expo(double x) {
    return exp(-x);
}

double inv_f_expo(double x) {
    return -log(x);
}

double f_norm(double x){
    return exp(-x*x/2.0);
}

double inv_f_norm(double x){
    return sqrt(-2*log(x));
}

