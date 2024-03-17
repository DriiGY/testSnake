#include<math.h>

internal int clamp(int min, int val, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}


struct {
    union {
        struct {
            f32 x;
            f32 y;
        };
        
        f32 e[2];
    };
} typedef v2;