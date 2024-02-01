// TODO: should removal be possible? it may be safer to just allow the user to overwrite
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdint.h>

#define register_lockfree_hash(keytype, valtype, name) \
    struct entry_pair_##name{ \
        keytype k; \
        /* this is need for ptr keytype, TODO: make sure this has intended behavior */ \
        valtype _Atomic v; \
    }; \
    struct entry_##name{ \
        struct entry_pair_##name kv; \
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
        struct entry_##name* last; \
        struct entry_##name* nil_entry; \
        struct entry_##name* new_e = malloc(sizeof(struct entry_##name)); \
        new_e->kv.k = key;\
        atomic_store(&new_e->kv.v, val); \
        new_e->next = NULL; \
        insert_overwrite: \
        /* dealing with first insertion */ \
        nil_entry = NULL; \
        if (atomic_compare_exchange_strong(&l->buckets[idx], &nil_entry, new_e)) { \
            return; \
        } \
        /* for the below, the initialization of ep does not need to be atomically loaded, this
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
            if (!memcmp(&ep->kv.k, &key, sizeof(keytype))) { \
                atomic_store(&ep->kv.v, val); \
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
        valtype ret; \
        uint16_t idx = l->hashfunc(key) % l->n_buckets; \
\
        memset(&ret, 0, sizeof(valtype)); \
        *found = 0; \
\
        for (struct entry_##name* ep = atomic_load(&l->buckets[idx]); ep; ep = atomic_load(&ep->next)) { \
            if (!memcmp(&ep->kv.k, &key, sizeof(keytype))){ \
                *found = 1; \
                /* interesting! this is only failing with pointer keys! value can be anything, pointer KEYS are bad */ \
                ret = atomic_load(&ep->kv.v); \
                return ret; \
            } \
        } \
        return ret; \
    } \
 \
    uint64_t fprint_##name(name* l, const char* fmtstr, FILE* fp) { \
        uint64_t sz = 0; \
        valtype v; \
        for (uint16_t i = 0; i < l->n_buckets; ++i) { \
            if (l->buckets[i]) { \
                fprintf(fp, "buckets[%i]:\n", i); \
            } \
            for (struct entry_##name* ep = atomic_load(&l->buckets[i]); ep; ep = atomic_load(&ep->next)) { \
                v = atomic_load(&ep->kv.v); \
                fprintf(fp, "  [%li] ", sz); \
                fprintf(fp, fmtstr, ep->kv.k, v); \
                fprintf(fp, "\n"); \
                ++sz; \
            } \
        } \
        fprintf(fp, "%li keys found\n", sz); \
        return sz; \
    }
