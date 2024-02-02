#include <pthread.h>
#include <assert.h>

#include <lfh.h>

uint16_t hfunc(int x) {
    return x;
}

uint16_t hfunc_ptr(int* x) {
    return (x) ? *x : 0;
}

uint16_t nwfunc(char* x) {
    return (x) ? *x : 0;
}

register_lockfree_hash(int, int, ashmap)
register_lockfree_hash(int*, int, ashmap_ptr)
register_lockfree_hash(int*, int*, ptrptr)
register_lockfree_hash(char*, float, networth)


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
void threadsafety_test(int n_buckets, int n_threads, int total_insertions){
    ashmap m;
    struct tsafe_test* tst;
    int ins_per_thread = total_insertions/n_threads;
    int val;
    uint64_t sz;
    _Bool found;

    init_ashmap(&m, n_buckets, hfunc);
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

    /*fprint_ashmap(&m, "%i: %i", stdout);*/
    for (int i = 0; i < total_insertions; ++i) {
        val = lookup_ashmap(&m, i, &found);
        assert(found && val == i);
    }

    fclose(stderr);
    sz = fprint_ashmap(&m, "%i: %i", stderr);

    assert(sz == (uint64_t)total_insertions);
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
    fprint_networth(&net, "\"%s\": %f", stdout);

    fprint_ashmap(&m, "%i: %i", stdout);
}

struct stest{
    char name[5];
    float value;
    int n;
};

uint16_t sthash(struct stest st){
    return ((uint16_t)st.value + st.n) * st.name[2];
}

register_lockfree_hash(struct stest, void*, strt)

void struct_test(){
    strt m;
    _Bool found;
    struct stest st;

    st.value = 94.21;
    st.n = 99;
    strcpy(st.name, "asher");

    init_strt(&m, 100, sthash);
    insert_strt(&m, st, NULL);

    lookup_strt(&m, st, &found);
    assert(found);
    ++st.name[0];
    lookup_strt(&m, st, &found);
    assert(!found);
    --st.name[0];
    lookup_strt(&m, st, &found);
    assert(found);
}

int main(){
    /* TODO: this fails, may be due to bad remainder when dividing n_ins/n_th */
    /*threadsafety_test(100, 6, 2000);*/
    threadsafety_test(100, 60, 60000);
    struct_test();
}
