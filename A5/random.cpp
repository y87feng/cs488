#include <cstdlib>
#include <stdlib.h>


float rand_float() {
    float x;
    do {
        x = (float) rand() / (RAND_MAX);
    } while (x == 1);
    return x;
}

// random float from 0.0f to 0.5f
float rand_float_half() {
    float x;
    do {
        x = (float) rand() / (RAND_MAX) / 2;
    } while (x == 1);
    return x;
}