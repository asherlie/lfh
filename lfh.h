// TODO: entries must be atomic, look below where i'm using CAS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdint.h>

#define INIT_LFH(keytype, valtype, name) \
    typedef struct entry_pair_##name{ \
        keytype k; \
        valtype v; \
    }entry_pair_##name; \
    struct entry_##name{ \
        _Atomic entry_pair_##name kv; \
        struct entry_##name* next; \
    }; \
 \
    struct bucket_##name{ \
        uint64_t sz, cap; \
        struct entry_##name* e; \
    }; \
 \
    typedef struct lfh_##name{ \
        uint16_t n_buckets; \
        size_t keysz, valsz; \
        struct bucket_##name* buckets; \
        uint16_t (*hashfunc)(keytype); \
    }name; \
 \
    void init_##name(name* l, uint16_t n_buckets, uint16_t (*hashfunc)(keytype)) { \
        l->keysz = sizeof(keytype); \
        l->valsz = sizeof(valtype); \
        l->n_buckets = n_buckets; \
        l->buckets = calloc(sizeof(struct bucket_##name), l->n_buckets); \
        /* \
         * for (uint16_t i = 0; i < l->n_buckets; ++i) { \
         *     l->buckets[i].sz = 0; \
         *     l->buckets[i].cap  = 0; \
         * } \
        */ \
        l->hashfunc = hashfunc; \
    } \
 \
    void insert_##name(name* l, keytype key, valtype val){ \
        uint16_t idx = l->hashfunc(key) % l->n_buckets; \
        struct bucket_##name* b = &l->buckets[idx]; \
        struct entry_pair_##name kv; \
        struct entry_##name* last; \
        struct entry_pair_##name kvcmp; \
        struct entry_##name* nil_entry = NULL; \
        struct entry_##name* new_e = malloc(sizeof(struct entry_##name)); \
        kv.k = key; \
        kv.v = val; \
        atomic_store(&new_e->kv, kv); \
        new_e->next = NULL; \
        insert_overwrite: \
        /* dealing with first insertion */ \
        if (atomic_compare_exchange_strong(&b->e, &nil_entry, new_e)) { \
            return; \
        } \
        for (struct entry_##name* e = b->e; e; e = e->next) { \
            last = e;\
            kvcmp = atomic_load(&e->kv); \
            if (!memcmp(&kvcmp.k, &key, sizeof(keytype))) { \
                atomic_store(&e->kv, kv); \
                return; \
            } \
        } \
        /* TODO: is this even valid? last->next is not atomic */ \
        if (!atomic_compare_exchange_strong(&last->next, &nil_entry, new_e)) { \
            goto insert_overwrite; \
        } \
    } \
\
    valtype lookup_##name(name* l, keytype key, _Bool* found) { \
        uint16_t idx = l->hashfunc(key) % l->n_buckets; \
        struct entry_pair_##name kv = {0}; \
        struct bucket_##name* b = &l->buckets[idx];  \
        *found = 0; \
\
        for (struct entry_##name* e = b->e; e; e = e->next) { \
            kv = atomic_load(&e->kv); \
            if (kv.k == key){ \
                *found = 1; \
                return kv.v; \
            } \
        } \
        return kv.v; \
    }
