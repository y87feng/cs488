#include <cstdlib>
#include <stdlib.h>


float rand_float() {
    float x;
    do {
        x = (float) rand() / (RAND_MAX);
    } while (x == 1);
    return x;
}