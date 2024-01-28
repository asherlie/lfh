#include "lfh.h"

uint16_t hfunc(int x) {
    return x;
}

uint16_t hfunc_ptr(int* x) {
    return (x) ? *x : 0;
}

INIT_LFH(int, int, ashmap)
INIT_LFH(int*, int, ashmap_ptr)

int main(){

    ashmap m;
    ashmap_ptr mp;
    int x = 9;

    _Bool found;
    init_ashmap(&m, 100, hfunc);
    init_ashmap_ptr(&mp, 100, hfunc_ptr);

    for (int i = 0; i < 1000; ++i) {
        insert_ashmap(&m, 100, i);
    }

    printf("ashmap: %i\n", lookup_ashmap(&m, 100, &found));

    insert_ashmap_ptr(&mp, NULL, 90);
    printf("ashmap_ptr: %i\n", lookup_ashmap_ptr(&mp, &x, &found));
}
