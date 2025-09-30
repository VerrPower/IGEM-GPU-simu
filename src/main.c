#include "stdbool.h"
#define N 3000
#define expo_test { #ifndef POGIRANDOM_H \
#error "please include PogiRandom.h"\
#endif \
PogiRandom rnd00; rnd_update_seed(&rnd00); rnd_init_ziggurat(); \
float arr00[N]; for (int i = 0; i < N; ++i) arr00[i] = rnd_expo(&rnd00);  \
float sum00 = 0, EX00 = 0, VarX00 = 0;    \
for (int i00 = 0; i00 < N; ++i00) sum00 += arr00[i00]; EX00 = sum00 / (float)N;  \
for (int i00 = 0; i00 < N; ++i00) VarX00 += (arr00[i00] - EX00) * (arr00[i00] - EX00); VarX00 /= (float)N;     \
printf("[normal] N = %d, sum = %.4f, mean = %.4f, variance = %.4f\n", N, sum00, EX00, VarX00);}

#include "Engine.h"

int main() {
    engine_init();
    engine_run();
    engine_close();
    return 0;
}
