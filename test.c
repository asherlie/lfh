#include "lfh.h"

uint16_t hfunc(int x) {
    return x;
}

    INIT_LFH(int, int, ashmap)
    INIT_LFH(int*, int, ashmap_ptr)
int main(){

    ashmap m;
    _Bool found;
    init_ashmap(&m, 100, hfunc);

    for (int i = 0; i < 1000; ++i) {
        insert_ashmap(&m, 100, i);
    }

    printf("%i\n", lookup_ashmap(&m, 100, &found));
}
