// TODO: entries must be atomic, look below where i'm using CAS
// TODO: should removal be possible? it may be safer to just allow the user to overwrite
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdint.h>

#define register_lockfree_hash(keytype, valtype, name) \
    /* TODO: only valtype must be _Atomic! this will silence our clang warnings */ \
    typedef struct entry_pair_##name{ \
        keytype k; \
        valtype v; \
    }entry_pair_##name; \
    struct entry_##name{ \
        _Atomic entry_pair_##name kv; \
        /* this syntax provides an _Atomic pointer to a non-_Atomic variable, see below as well. */ \
        struct entry_##name* _Atomic next; \
    }; \
 \
    typedef struct lfh_##name{ \
        uint16_t n_buckets; \
        /* TODO: think this through, should this be * _Atomic * _Atomic buckets?
         * I think current behavior is correct - a pointer to an atomic pointer, the outer pointer
         * needs not be atomic, we're only setting the inner pointer atomically upon insertion of
         * first element of a new bucket
         */ \
        struct entry_##name* _Atomic * buckets; \
        uint16_t (*hashfunc)(keytype); \
    }name; \
 \
    void init_##name(name* l, uint16_t n_buckets, uint16_t (*hashfunc)(keytype)) { \
        l->n_buckets = n_buckets; \
        l->buckets = calloc(sizeof(struct entry_##name*), l->n_buckets); \
        l->hashfunc = hashfunc; \
    } \
 \
    void insert_##name(name* l, keytype key, valtype val){ \
        uint16_t idx = l->hashfunc(key) % l->n_buckets; \
        struct entry_pair_##name kv; \
        struct entry_##name* last; \
        struct entry_pair_##name kvcmp; \
        struct entry_##name* nil_entry; \
        struct entry_##name* new_e = malloc(sizeof(struct entry_##name)); \
        kv.k = key; \
        kv.v = val; \
        atomic_store(&new_e->kv, kv); \
        new_e->next = NULL; \
        insert_overwrite: \
        /* dealing with first insertion */ \
        nil_entry = NULL; \
        /*
         * if (atomic_compare_exchange_strong(&e, &nil_entry, new_e)) { \
        */ \
        if (atomic_compare_exchange_strong(&l->buckets[idx], &nil_entry, new_e)) { \
            return; \
        } \
        /* i believe i've brken threadsafety, ep->next isn't atomic... what are the implications of this?
         * i think that the iterator should be _Atomic. this can be atomic_load()ed just for e = e->next
         *
         * thinking through this - the above is perfect, sets entry atomically if NULL
         *
         * for the below, the initialization of ep does not need to be atomically loaded, this
         * will never change once it's non-NULL
         * ep->next can change, however, when last->next is updated below
         * entries are never freed/deleted, pointers never changed. this means i can likely not worry
         * about a new LL path being created. i can just atomic_load() both as is already done below
         *
         * my concern is that neither of these atomic_load()s are necessary with my current _Atomic usage
         *
         * should i just be using _Atomic (blah) regular style?
         * logic is sound but is definition?
         * TODO: do i need an atomic object or just atomic pointer to a regular object?
         * it seems that behavior doesn't change when either one of these atomic_load()s is removed
         *      also... it seems unintuitive that the compiler would even allow removing these
         */ \
        for (struct entry_##name* ep = atomic_load(&l->buckets[idx]); ep; ep = atomic_load(&ep->next)) { \
            last = ep; \
            kvcmp = atomic_load(&ep->kv); \
            if (!memcmp(&kvcmp.k, &key, sizeof(keytype))) { \
                atomic_store(&ep->kv, kv); \
                return; \
            } \
        } \
        nil_entry = NULL; \
        if (!atomic_compare_exchange_strong(&last->next, &nil_entry, new_e)) { \
            goto insert_overwrite; \
        } \
    } \
\
    valtype lookup_##name(name* l, keytype key, _Bool* found) { \
        uint16_t idx = l->hashfunc(key) % l->n_buckets; \
        struct entry_pair_##name kv = {0}; \
        *found = 0; \
\
        for (struct entry_##name* ep = atomic_load(&l->buckets[idx]); ep; ep = atomic_load(&ep->next)) { \
            kv = atomic_load(&ep->kv); \
            if (!memcmp(&kv.k, &key, sizeof(keytype))){ \
                *found = 1; \
                return kv.v; \
            } \
        } \
        return kv.v; \
    } \
 \
    uint64_t fprint_##name(name* l, const char* fmtstr, FILE* fp) { \
        uint64_t sz = 0; \
        struct entry_pair_##name kv; \
        for (uint16_t i = 0; i < l->n_buckets; ++i) { \
            if (l->buckets[i]) { \
                fprintf(fp, "buckets[%i]:\n", i); \
            } \
            for (struct entry_##name* ep = atomic_load(&l->buckets[i]); ep; ep = atomic_load(&ep->next)) { \
                kv = atomic_load(&ep->kv); \
                fprintf(fp, "  [%li] ", sz); \
                fprintf(fp, fmtstr, kv.k, kv.v); \
                fprintf(fp, "\n"); \
                ++sz; \
            } \
        } \
        fprintf(fp, "%li keys found\n", sz); \
        return sz; \
    }
