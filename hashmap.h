#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#define HASHMAP_INITIAL_CAPACITY 32

#if H_DEBUG
#define H_ASSERT(pred, text)                                               \
    {                                                                      \
        if (!(pred))                                                       \
        {                                                                  \
            fprintf(stderr, "ASSERT: %s %s %d", text, __FILE__, __LINE__); \
            *((int *)0) = 0;                                               \
        }                                                                  \
    }
#else
#define H_ASSERT(pred, text) pred
#endif

typedef uint64_t h_u64;
typedef uint8_t h_bool;
typedef uint32_t h_u32;
typedef size_t h_size;
#define H_SUCCESS (1)
#define H_TRUE (1)
#define H_FAILURE (0)
#define H_FALSE (0)

h_u64 malloc_ctr = 0;

void *counter_malloc(size_t size)
{
    malloc_ctr += size;
    return malloc(size);
}
enum h_result
{
    NO_ERROR,
    MAP_FULL,
    SAME_KEY,
    NULL_PTR,
    FOUND_HIGHER_PSL,
    EMPTY_BUCKET,
    EXCEEDED_MAP_BOUNDS,
    UNKNOWN_ERROR = 0xFFFF
};

static inline h_bool is_power_of_two(h_u32 x)
{
    return (x & (x - 1)) == 0;
}

static inline h_u32 compute_next_highest_power_of_two(h_u32 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

const int tab32[32] = {
    0, 9, 1, 10, 13, 21, 2, 29,
    11, 14, 16, 18, 22, 25, 3, 30,
    8, 12, 20, 28, 15, 17, 24, 7,
    19, 27, 23, 6, 26, 5, 4, 31};

int log_2_h_u32(h_u32 value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return tab32[(h_u32)(value * 0x07C4ACDD) >> 27];
}

static inline h_u32 max_psl(h_u32 n_buckets)
{
    // return (h_u32)log2(n_buckets);
    return log_2_h_u32(n_buckets);
}

static inline h_u64 power_2_mod(h_u64 a, h_u64 b)
{
    return a & (b - 1);
}

#define HASH_FUNCTION(name, type) h_u64 name(type *to_hash)
#define HASH_EQUALS(name, type) h_bool name(type *a, type *b)

// TODO handle tie-breakers. Currently we just move on, but this isn't optimal (i think)
#define HASHMAP_INIT(name, key_type, val_type, __hash_func, __equals_func)                                     \
    typedef HASH_FUNCTION(h_hashfunc_##name, key_type);                                                        \
    typedef HASH_EQUALS(h_equalfunc_##name, key_type);                                                         \
    struct bucket_##name                                                                                       \
    {                                                                                                          \
        h_bool full;                                                                                           \
        h_u32 psl;                                                                                             \
        key_type key;                                                                                          \
        val_type val;                                                                                          \
    };                                                                                                         \
                                                                                                               \
    static inline h_result fill_bucket_##name(bucket_##name *bucket, key_type key, val_type val)               \
    {                                                                                                          \
        H_ASSERT(!bucket->full, "Cannot fill an already full bucket");                                         \
        bucket->full = true;                                                                                   \
        bucket->psl = 0;                                                                                       \
        bucket->key = key;                                                                                     \
        bucket->val = val;                                                                                     \
        return NO_ERROR;                                                                                       \
    }                                                                                                          \
                                                                                                               \
    struct h_map_##name                                                                                        \
    {                                                                                                          \
        h_u32 n_buckets;                                                                                       \
        h_u32 max_psl;                                                                                         \
        h_u32 buckets_used;                                                                                    \
        bucket_##name *buckets;                                                                                \
        h_hashfunc_##name *hash_func;                                                                          \
        h_equalfunc_##name *equal_func;                                                                        \
    };                                                                                                         \
                                                                                                               \
    static inline h_map_##name h_init_##name(h_u32 capacity = HASHMAP_INITIAL_CAPACITY)                        \
    {                                                                                                          \
        h_map_##name ret = {};                                                                                 \
        if (!is_power_of_two(capacity))                                                                        \
        {                                                                                                      \
            capacity = compute_next_highest_power_of_two(capacity);                                            \
        }                                                                                                      \
        ret.n_buckets = capacity;                                                                              \
        ret.buckets = (bucket_##name *)counter_malloc(sizeof(bucket_##name) * ret.n_buckets);                  \
        memset(ret.buckets, 0, ret.n_buckets * sizeof(bucket_##name));                                         \
        ret.hash_func = __hash_func;                                                                           \
        ret.equal_func = __equals_func;                                                                        \
        ret.max_psl = max_psl(ret.n_buckets);                                                                  \
        return ret;                                                                                            \
    }                                                                                                          \
                                                                                                               \
    static inline h_result swap_##name##_buckets(bucket_##name *a, bucket_##name *b)                           \
    {                                                                                                          \
        if (!a || !b)                                                                                          \
        {                                                                                                      \
            return NULL_PTR;                                                                                   \
        }                                                                                                      \
        bucket_##name temp = *a;                                                                               \
        *a = *b;                                                                                               \
        *b = temp;                                                                                             \
        return NO_ERROR;                                                                                       \
    }                                                                                                          \
                                                                                                               \
    static h_result probe_##name(h_map_##name *map,                                                            \
                                 h_u64 hash_index,                                                             \
                                 key_type key,                                                                 \
                                 val_type val,                                                                 \
                                 h_u64 *index_inserted = 0,                                                    \
                                 h_bool *shuffled = 0,                                                         \
                                 h_bool *grew = 0);                                                            \
                                                                                                               \
    static inline h_u64 compute_index_##name(h_map_##name *map, key_type *key)                                 \
    {                                                                                                          \
        return (map->hash_func(key) & (map->n_buckets - 1));                                                   \
    }                                                                                                          \
                                                                                                               \
    static h_result grow_map_##name(h_map_##name *map)                                                         \
    {                                                                                                          \
        h_u32 prev_size = map->n_buckets;                                                                      \
        if (!is_power_of_two(map->n_buckets))                                                                  \
        {                                                                                                      \
            map->n_buckets = compute_next_highest_power_of_two(map->n_buckets);                                \
        }                                                                                                      \
        else                                                                                                   \
        {                                                                                                      \
            map->n_buckets *= 2;                                                                               \
        }                                                                                                      \
        bucket_##name *old_buckets = map->buckets;                                                             \
                                                                                                               \
        map->buckets = (bucket_##name *)malloc(sizeof(bucket_##name) * map->n_buckets);                        \
        memset(map->buckets, 0, map->n_buckets * sizeof(bucket_##name));                                       \
        map->buckets_used = 0;                                                                                 \
        for (h_u32 i = 0; i < prev_size; i++)                                                                  \
        {                                                                                                      \
            bucket_##name *b = &old_buckets[i];                                                                \
            if (b->full)                                                                                       \
            {                                                                                                  \
                h_u64 new_hash = compute_index_##name(map, &b->key);                                           \
                h_bool grew = false;                                                                           \
                probe_##name(map, new_hash, b->key, b->val, 0, 0, &grew);                                      \
            }                                                                                                  \
        }                                                                                                      \
        free(old_buckets);                                                                                     \
        return NO_ERROR;                                                                                       \
    }                                                                                                          \
    static inline h_u32 count_full_buckets_##name(h_map_##name *map)                                           \
    {                                                                                                          \
        h_u32 ctr = 0;                                                                                         \
        for (int i = 0; i < map->n_buckets; i++)                                                               \
        {                                                                                                      \
            if (map->buckets[i].full)                                                                          \
            {                                                                                                  \
                ctr++;                                                                                         \
            }                                                                                                  \
        }                                                                                                      \
        return ctr;                                                                                            \
    }                                                                                                          \
                                                                                                               \
    static h_result probe_##name(h_map_##name *map,                                                            \
                                 h_u64 hash_index,                                                             \
                                 key_type key,                                                                 \
                                 val_type val,                                                                 \
                                 h_u64 *index_inserted,                                                        \
                                 h_bool *shuffled,                                                             \
                                 h_bool *grew)                                                                 \
    {                                                                                                          \
        bucket_##name temp_bucket;                                                                             \
        temp_bucket.full = 1;                                                                                  \
        temp_bucket.psl = 0;                                                                                   \
        temp_bucket.key = key;                                                                                 \
        temp_bucket.val = val;                                                                                 \
        h_u64 probe_position = hash_index;                                                                     \
        h_result ret = UNKNOWN_ERROR;                                                                          \
        while (temp_bucket.psl < map->max_psl && probe_position < map->n_buckets)                              \
        {                                                                                                      \
            bucket_##name *target_bucket = &map->buckets[probe_position];                                      \
            if (target_bucket->full)                                                                           \
            {                                                                                                  \
                h_bool same_key = map->equal_func(&target_bucket->key, &temp_bucket.key);                      \
                if (same_key == H_TRUE)                                                                        \
                {                                                                                              \
                    return SAME_KEY;                                                                           \
                }                                                                                              \
                if (target_bucket->psl < temp_bucket.psl)                                                      \
                {                                                                                              \
                    h_result swap_result = swap_##name##_buckets(target_bucket, &temp_bucket);                 \
                    if (swap_result != NO_ERROR)                                                               \
                    {                                                                                          \
                        return swap_result;                                                                    \
                    }                                                                                          \
                    if (index_inserted && map->equal_func(&target_bucket->key, &key))                          \
                    {                                                                                          \
                        *index_inserted = probe_position;                                                      \
                        if (shuffled)                                                                          \
                        {                                                                                      \
                            *shuffled = true;                                                                  \
                        }                                                                                      \
                    }                                                                                          \
                }                                                                                              \
                else                                                                                           \
                {                                                                                              \
                    temp_bucket.psl++;                                                                         \
                }                                                                                              \
            }                                                                                                  \
            else                                                                                               \
            {                                                                                                  \
                memcpy(target_bucket, &temp_bucket, sizeof(temp_bucket));                                      \
                map->buckets_used++;                                                                           \
                if (shuffled)                                                                                  \
                {                                                                                              \
                    *shuffled = false;                                                                         \
                }                                                                                              \
                return NO_ERROR;                                                                               \
            }                                                                                                  \
            probe_position++;                                                                                  \
        }                                                                                                      \
                                                                                                               \
        if (probe_position >= map->n_buckets || temp_bucket.psl >= map->max_psl)                               \
        {                                                                                                      \
            if (grew)                                                                                          \
            {                                                                                                  \
                *grew = true;                                                                                  \
            }                                                                                                  \
            grow_map_##name(map);                                                                              \
            h_u64 target_idx = compute_index_##name(map, &temp_bucket.key);                                    \
            h_bool grew_twice = false;                                                                         \
            h_result res = probe_##name(map, target_idx, temp_bucket.key, temp_bucket.val, 0, 0, &grew_twice); \
            H_ASSERT(grew_twice == false, "h_map shouldn't be growing twice");                                 \
            return res;                                                                                        \
        }                                                                                                      \
        return ret;                                                                                            \
    }                                                                                                          \
    static h_result h_put_##name(h_map_##name *map, key_type key, val_type val)                                \
    {                                                                                                          \
        if (map->buckets_used >= map->n_buckets)                                                               \
        {                                                                                                      \
            grow_map_##name(map);                                                                              \
            map->max_psl = max_psl(map->n_buckets);                                                            \
        }                                                                                                      \
        h_u64 index = compute_index_##name(map, &key);                                                         \
        h_result probe_result = UNKNOWN_ERROR;                                                                 \
        probe_result = probe_##name(map, index, key, val);                                                     \
        return probe_result;                                                                                   \
    }                                                                                                          \
                                                                                                               \
    static inline h_result hashmap_insert_##name(h_map_##name *map, key_type key, val_type val)                \
    {                                                                                                          \
        return h_put_##name(map, key, val);                                                                    \
    }                                                                                                          \
                                                                                                               \
    static h_result h_retrieve_##name(h_map_##name *map, key_type key, val_type *o_val)                        \
    {                                                                                                          \
        h_u64 index = compute_index_##name(map, &key);                                                         \
        h_u32 max_psl_dist = max_psl(map->n_buckets);                                                          \
        for (h_u32 i = index; i < map->n_buckets && i < max_psl_dist + index; i++)                             \
        {                                                                                                      \
            bucket_##name *b = &map->buckets[i];                                                               \
            if (b->full)                                                                                       \
            {                                                                                                  \
                if (b->psl > i)                                                                                \
                {                                                                                              \
                    o_val = NULL;                                                                              \
                    return FOUND_HIGHER_PSL;                                                                   \
                }                                                                                              \
                else                                                                                           \
                {                                                                                              \
                    if (map->equal_func(&b->key, &key))                                                        \
                    {                                                                                          \
                        o_val = &b->val;                                                                       \
                        return NO_ERROR;                                                                       \
                    }                                                                                          \
                }                                                                                              \
            }                                                                                                  \
            else                                                                                               \
            {                                                                                                  \
                o_val = NULL;                                                                                  \
                return EMPTY_BUCKET;                                                                           \
            }                                                                                                  \
        }                                                                                                      \
        return EXCEEDED_MAP_BOUNDS;                                                                            \
    }                                                                                                          \
                                                                                                               \
    static inline h_bool h_free_##name(h_map_##name *map)                                                      \
    {                                                                                                          \
        if (map->buckets)                                                                                      \
        {                                                                                                      \
            free(map->buckets);                                                                                \
            *map = {};                                                                                         \
        }                                                                                                      \
        else                                                                                                   \
        {                                                                                                      \
            return H_FAILURE;                                                                                  \
        }                                                                                                      \
    }

#endif