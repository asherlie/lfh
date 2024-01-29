#include <pthread.h>

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


struct tsafe_test{
    int insertions;
    int thread_base;
    ashmap* m;
};
void* insert_test_thread(void* arg){
    struct tsafe_test* tst = arg;
    for (int i = 0; i < tst->insertions; ++i) {
        insert_ashmap(tst->m, tst->thread_base + i, tst->thread_base + i);
    }
    return NULL;
}
// n_threads are used to insert simulatneously into an int: int map 
// each
void threadsafety_test(int n_threads, int total_insertions){
    ashmap m;
    struct tsafe_test* tst;
    int ins_per_thread = total_insertions/n_threads;

    init_ashmap(&m, 100, hfunc);
    pthread_t* pth = malloc(sizeof(pth)*n_threads);

    for (int i = 0; i < n_threads; ++i) {
        tst = malloc(sizeof(struct tsafe_test));
        tst->m = &m;
        tst->thread_base = i*ins_per_thread;
        tst->insertions = ins_per_thread;
        pthread_create(pth+i, NULL, insert_test_thread, tst);
    }

    for (int i = 0; i < n_threads; ++i) {
        pthread_join(pth[i], NULL);
        printf("joined thread %i\n", i);
    }

    print_ashmap(&m, "%i: %i");
}

void single_thread_tests(){

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

int main(){
    threadsafety_test(10, 30);
}
