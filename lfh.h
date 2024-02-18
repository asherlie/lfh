// TODO: should removal be possible? it may be safer to just allow the user to overwrite
// TODO: include entry_##name_foreach - this will allow programmers to take advantage
// of their hash functions to use the value linked list as the threadsafe lock free LL it is
// for example, the programmer of a reminder program could have a hashing function that hashes on 
// month and day, but not contents of reminder
// the foreach #define would let the user take advantage of the structure of lfhash
// TODO: maybe also define a foreach_key
// TODO: foreach_entry_##name() should be used to implement fprint_name() and lookup_##name()
//
// okay, adding it now. i need to move hashfunc def to be in register_lockfree_hash()
// it doesn't make sense to have it at the init level, nvm it does. but i like it more being separate
// this must be set there so that we can define a lookup hash function, NVM... we can still do this assuming
// that init() is always called first
// not the safest though
// TODO: consider moving hashfunc param to register_lockfree_hash()param to register_lockfree_hash()
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdint.h>

// try defining this first, then redefining it in below? nah won't work. damn
// i'm stuck because i can't make nested macros.
// i want to just #define it below using name and valtype, etc. think tomorrow about this
/*
 * #define FOREACH_ENTRY(l, k, iter) \
 *     uint16_t idx = l->hashfunc(k) % l->n_buckets; \
 *     for (valtype i = 
*/
//#define FOREACH_ENTRY_##NAME(a) a

/*
 * #define foreach(l, k, i) \
 *     uint16_t idx = l->hashfunc(k) % l->n_buckets; 
*/
    //for (int idx = 0; i < 10; ++i
    //

// TODO: should iterk be a pointer?
// this would be faster, maybe add the option for this with a second macro, this can be set to NULL
// and it'll be the most base level
// TODO: replace existing with these macros?
// TODO: check that this is type strict, try passing wrong types and seeing if gcc complains
// TODO: pick one of the below, i don't like the _idx() built out of idx_kptr()
// ugh, nvm, i really like the idea actually, especially bc it's so critical to have the pointer version
// but the pointer version is so unintuitive for use outside this header
//UGH. foreach* should not be used for most of this file, it ALWAYS loads value, so it's better suited to the user

/*
 * this should be multiple macros - foreach() just exposes a variable ep using idx
 * then we have combinations of key and value
*/

#define foreach_entry_idx_kptr(name, l, idx, iterkptr, iterv) \
    for (struct entry_##name* _internal_foreach_e = atomic_load(&l->buckets[idx]); _internal_foreach_e; _internal_foreach_e = atomic_load(&_internal_foreach_e->next)){ \
        /* kv.k must be cast to a  */ \
        iterkptr = (void*)&_internal_foreach_e->kv.k; \
        iterv = atomic_load(&_internal_foreach_e->kv.v);

// TODO: should i implement foreach_entry_idx() without key pointer?
// this can be done using keypointer like this:
/*
 * #define foreach_entry_idx(name, l, idx, iterk, iterv) \
 *     for (struct entry_##name* e = atomic_load(&l->buckets[idx]); e; e = atomic_load(&e->next)){ \
 *         iterk = e->kv.k; \
 *         iterv = atomic_load(&e->kv.v);
*/

// TODO: can just use a void*, this is a bit easier becuse we can just pretend that entry_pair* is a pointer to just k, it's at beginning anyway
// TODO: is it safer to use a void* and memcpy() into spoof_e.k? this ensures no weirdness with data format, what if there's padding?
// TODO: it probably makes more sense to just duplicate code between this and _kptr() because we don't waste the bytes of spoof_e.v
// that way we can just set iterk = kv.k
// actually, we're not even wasting space really because we're just using a pointer, still though, a pointer is larger than let's say, an int
#define foreach_entry_idx(name, l, idx, iterk, iterv) \
    struct entry_pair_##name* _internal_foreach_spoof_e; \
    foreach_entry_idx_kptr(name, l, idx, _internal_foreach_spoof_e, iterv) \
        iterk = _internal_foreach_spoof_e->k;

#define foreach_entry(name, l, k, iterk, iterv) \
    uint16_t _internal_foreach_idx = (l)->hashfunc(k) % (l)->n_buckets; \
    foreach_entry_idx(name, l, _internal_foreach_idx, iterk, iterv)

/* new def */

#define HASH(l, key) l->hashfunc(key) % l->n_buckets

#define _foreach_entry_idx(name, l, idx, iter_entry) \
    for (struct entry_##name* iter_entry = atomic_load(&l->buckets[idx]); iter_entry; iter_entry = atomic_load(&iter_entry->next)) {

// TODO: rename ep so it's less likely to be already defined
#define _foreach_entry_k(name, l, key, iterk) \
    uint16_t idx = HASH(l, key); \
    struct entry_pair_##name* ep; \
    _foreach_entry_idx(name, l, idx, ep) \
        iterk = ep->kv.k; 

#define _foreach_entry_kptr(name, l, key, iterkptr) \
    uint16_t idx = HASH(l, key); \
    _foreach_entry_idx(name, l, idx, ep) \
        iterkptr = &ep->kv.k; 

#define _foreach_entry_v(name, l, key, iterv) \
    uint16_t idx = HASH(l, key); \
    _foreach_entry_idx(name, l, idx, ep) \
        iterv = atomic_load(&ep->kv.v);

#define _foreach_entry_kv(name, l, key, iterk, iterv) \
    _foreach_entry_k(name, l, key, iterk) \
        iterv = atomic_load(&ep->kv.v);
        
#define _foreach_entry_kptrv(name, l, key, iterkptr, iterv) \
    _foreach_entry_kptr(name, l, key, iterkptr) \
        iterv = atomic_load(&ep->kv.v);

#define _foreach_entry_e_k(name, l, key, iterk, iter_entry) \
    uint16_t idx = HASH(l, key); \
    _foreach_entry_idx(name, l, idx, iter_entry) \
        iterk = iter_entry->kv.k; 

#define _foreach_entry_e_kptr(name, l, key, iterkptr, iter_entry) \
    uint16_t idx = HASH(l, key); \
    _foreach_entry_idx(name, l, idx, iter_entry) \
        iterkptr = &iter_entry->kv.k; 

#define _foreach_entry_e_v(name, l, key, iterv, iter_entry) \
    uint16_t idx = HASH(l, key); \
    _foreach_entry_idx(name, l, idx, iter_entry) \
        iterv = atomic_load(&iter_entry->kv.v);

#define _foreach_entry_e_kv(name, l, key, iterk, iterv, iter_entry) \
    _foreach_entry_e_k(name, l, key, iterk, iter_entry) \
        iterv = atomic_load(&iter_entry->kv.v);
        
#define _foreach_entry_e_kptrv(name, l, key, iterkptr, iterv, iter_entry) \
    _foreach_entry_e_kptr(name, l, key, iterkptr, iter_entry) \
        iterv = atomic_load(&iter_entry->kv.v);
    
    

#define register_lockfree_hash(keytype, valtype, name) \
    struct entry_pair_##name{ \
        keytype k; \
        /* this is needed for ptr keytype, TODO: make sure this has intended behavior */ \
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
        struct entry_##name* last; \
        struct entry_##name* nil_entry; \
        struct entry_##name* new_e = malloc(sizeof(struct entry_##name)); \
        new_e->kv.k = key;\
        atomic_store(&new_e->kv.v, val); \
        new_e->next = NULL; \
        insert_overwrite: \
        /* dealing with first insertion */ \
        nil_entry = NULL; \
        /* there is no advantage to precomputing idx, two branches will never both be reached */ \
        if (atomic_compare_exchange_strong(&l->buckets[l->hashfunc(key) % l->n_buckets], &nil_entry, new_e)) { \
            return; \
        } \
        /* TODO: why does compiler allow removal of atomic_load()s below? */ \
        keytype* kptr; \
        _foreach_entry_kptr(name, l, key, kptr) \
        /* _foreach_entry_idx(name, l, l->hashfunc(key) % l->n_buckets, ep)*/\
        /* for (struct entry_##name* ep = atomic_load(&l->buckets[l->hashfunc(key) % l->n_buckets]); ep; ep = atomic_load(&ep->next)) { */\
            last = ep; \
            if (!memcmp(kptr, &key, sizeof(keytype))) { \
            /* if (!memcmp(&ep->kv.k, &key, sizeof(keytype))) { */\
                atomic_store(&ep->kv.v, val); \
                return; \
            }\
        }\
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
        for (struct entry_##name* ep = atomic_load(&l->buckets[idx]); ep; ep = atomic_load(&ep->next)){ \
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
    //#define FOREACH_ENTRY_##NAME(a) a
/*
 * 
 * 
 * 
 *  * #define FOREACH_ENTRY(l, k, iter) \
 *  *     uint16_t idx = l->hashfunc(k) % l->n_buckets; \
 *  *     for (valtype i = 
*/
