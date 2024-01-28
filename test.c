#include "lfh.h"

uint16_t hfunc(int x) {
    return x;
}

uint16_t hfunc_ptr(int* x) {
    return (x) ? *x : 0;
}

uint16_t nwfunc(char* x) {
    return (x) ? *x : 0;
}

INIT_LFH(int, int, ashmap)
INIT_LFH(int*, int, ashmap_ptr)
INIT_LFH(int*, int*, ptrptr)
INIT_LFH(char*, float, networth)

int main(){

    ashmap m;
    ashmap_ptr mp;
    ptrptr pp;
    networth net;

    _Bool found;
    init_ashmap(&m, 100, hfunc);
    init_ashmap_ptr(&mp, 100, hfunc_ptr);
    init_ptrptr(&pp, 100, hfunc_ptr);
    init_networth(&net, 100, nwfunc);

    for (int i = 0; i < 1000; ++i) {
        insert_ashmap(&m, 100+i, i);
    }

    printf("ashmap: %i\n", lookup_ashmap(&m, 100, &found));

    insert_ashmap_ptr(&mp, NULL, 90);
    printf("ashmap_ptr: %i\n", lookup_ashmap_ptr(&mp, NULL, &found));

    insert_ptrptr(&pp, NULL, NULL);
    lookup_ptrptr(&pp, NULL, &found);

    insert_networth(&net, "john", 99.3);
    insert_networth(&net, "john", 99.399);
    insert_networth(&net, "John", 99.399);
    insert_networth(&net, "john", -1);
    print_networth(&net, "\"%s\": %f");

    print_ashmap(&m, "%i: %i");
}
