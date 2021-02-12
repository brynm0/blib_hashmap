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
#define HASHMAP_INIT(name, key_type, val_type, __hash_func, __equals_func)                      \
    typedef HASH_FUNCTION(h_hashfunc_##name, key_type);                                         \
    typedef HASH_EQUALS(h_equalfunc_##name, key_type);                                          \
                                                                                                \
    struct h_map_##name                                                                         \
    {                                                                                           \
        h_u32 n_buckets;                                                                        \
        h_u32 max_psl;                                                                          \
        h_u32 buckets_used;                                                                     \
        h_bool *fulls;                                                                          \
        h_u32 *psls;                                                                            \
        key_type *keys;                                                                         \
        val_type *vals;                                                                         \
        h_hashfunc_##name *hash_func;                                                           \
        h_equalfunc_##name *equal_func;                                                         \
    };                                                                                          \
                                                                                                \
    static inline void allocate_and_set_buffers(h_map_##name *map)                              \
    {                                                                                           \
        map->fulls = (h_bool *)counter_malloc(sizeof(h_bool) * map->n_buckets);                 \
        map->psls = (h_u32 *)counter_malloc(sizeof(h_u32) * map->n_buckets);                    \
        map->keys = (key_type *)counter_malloc(sizeof(key_type) * map->n_buckets);              \
        map->vals = (val_type *)counter_malloc(sizeof(val_type) * map->n_buckets);              \
                                                                                                \
        memset(map->fulls, 0, map->n_buckets * sizeof(h_bool));                                 \
        memset(map->psls, 0, map->n_buckets * sizeof(h_u32));                                   \
        memset(map->keys, 0, map->n_buckets * sizeof(key_type));                                \
        memset(map->vals, 0, map->n_buckets * sizeof(val_type));                                \
    }                                                                                           \
                                                                                                \
    static inline h_map_##name h_init_##name(h_u32 capacity = HASHMAP_INITIAL_CAPACITY)         \
    {                                                                                           \
        h_map_##name ret = {};                                                                  \
        if (!is_power_of_two(capacity))                                                         \
        {                                                                                       \
            capacity = compute_next_highest_power_of_two(capacity);                             \
        }                                                                                       \
        ret.n_buckets = capacity;                                                               \
        allocate_and_set_buffers(&ret);                                                         \
                                                                                                \
        ret.hash_func = __hash_func;                                                            \
        ret.equal_func = __equals_func;                                                         \
        ret.max_psl = max_psl(ret.n_buckets);                                                   \
        return ret;                                                                             \
    }                                                                                           \
                                                                                                \
    static inline h_result swap_##name##_buckets(h_map_##name *map, h_u32 a_id, h_u32 b_id)     \
    {                                                                                           \
        h_bool temp_full = map->fulls[a_id];                                                    \
        map->fulls[a_id] = map->fulls[b_id];                                                    \
        map->fulls[b_id] = temp_full;                                                           \
                                                                                                \
        h_u32 temp_psl = map->psls[a_id];                                                       \
        map->psls[a_id] = map->psls[b_id];                                                      \
        map->psls[b_id] = temp_psl;                                                             \
                                                                                                \
        key_type temp_key = map->keys[a_id];                                                    \
        map->keys[a_id] = map->keys[b_id];                                                      \
        map->keys[b_id] = temp_key;                                                             \
                                                                                                \
        val_type temp_val = map->vals[a_id];                                                    \
        map->vals[a_id] = map->vals[b_id];                                                      \
        map->vals[b_id] = temp_val;                                                             \
                                                                                                \
        return NO_ERROR;                                                                        \
    }                                                                                           \
                                                                                                \
    static h_result probe_##name(h_map_##name *map,                                             \
                                 h_u64 hash_index,                                              \
                                 key_type key,                                                  \
                                 val_type val,                                                  \
                                 h_u64 *index_inserted = 0,                                     \
                                 h_bool *shuffled = 0,                                          \
                                 h_bool *grew = 0);                                             \
                                                                                                \
    static inline h_u64 compute_index_##name(h_map_##name *map, key_type *key)                  \
    {                                                                                           \
        return (map->hash_func(key) & (map->n_buckets - 1));                                    \
    }                                                                                           \
                                                                                                \
    static h_result grow_map_##name(h_map_##name *map)                                          \
    {                                                                                           \
        h_u32 prev_size = map->n_buckets;                                                       \
        if (!is_power_of_two(map->n_buckets))                                                   \
        {                                                                                       \
            map->n_buckets = compute_next_highest_power_of_two(map->n_buckets);                 \
        }                                                                                       \
        else                                                                                    \
        {                                                                                       \
            map->n_buckets *= 2;                                                                \
        }                                                                                       \
        h_bool *old_fulls = map->fulls;                                                         \
        key_type *old_keys = map->keys;                                                         \
        val_type *old_vals = map->vals;                                                         \
                                                                                                \
        allocate_and_set_buffers(map);                                                          \
        map->buckets_used = 0;                                                                  \
        for (h_u32 i = 0; i < prev_size; i++)                                                   \
        {                                                                                       \
            if (old_fulls[i])                                                                   \
            {                                                                                   \
                h_u64 new_hash = compute_index_##name(map, &old_keys[i]);                       \
                h_bool grew = false;                                                            \
                probe_##name(map, new_hash, old_keys[i], old_vals[i], 0, 0, 0);                 \
            }                                                                                   \
        }                                                                                       \
        free(old_fulls);                                                                        \
        free(old_keys);                                                                         \
        free(old_vals);                                                                         \
        return NO_ERROR;                                                                        \
    }                                                                                           \
                                                                                                \
    static h_result probe_##name(h_map_##name *map,                                             \
                                 h_u64 hash_index,                                              \
                                 key_type key,                                                  \
                                 val_type val,                                                  \
                                 h_u64 *index_inserted,                                         \
                                 h_bool *shuffled,                                              \
                                 h_bool *grew)                                                  \
    {                                                                                           \
        h_u64 probe_position = hash_index;                                                      \
        h_result ret = UNKNOWN_ERROR;                                                           \
        h_u32 psl_curr = 0;                                                                     \
        while (psl_curr < map->max_psl && probe_position < map->n_buckets)                      \
        {                                                                                       \
            if (map->fulls[probe_position])                                                     \
            {                                                                                   \
                h_bool same_key = map->equal_func(&map->keys[probe_position], &key);            \
                if (same_key == H_TRUE)                                                         \
                {                                                                               \
                    return SAME_KEY;                                                            \
                }                                                                               \
                if (map->psls[probe_position] < psl_curr)                                       \
                {                                                                               \
                    h_u32 temp_psl = psl_curr;                                                  \
                    psl_curr = map->psls[probe_position];                                       \
                    map->psls[probe_position] = temp_psl;                                       \
                                                                                                \
                    key_type temp_key = key;                                                    \
                    key = map->keys[probe_position];                                            \
                    map->keys[probe_position] = key;                                            \
                                                                                                \
                    val_type temp_val = val;                                                    \
                    val = map->vals[probe_position];                                            \
                    map->vals[probe_position] = temp_val;                                       \
                }                                                                               \
                else                                                                            \
                {                                                                               \
                    psl_curr++;                                                                 \
                }                                                                               \
            }                                                                                   \
            else                                                                                \
            {                                                                                   \
                map->fulls[probe_position] = 1;                                                 \
                map->psls[probe_position] = psl_curr;                                           \
                map->keys[probe_position] = key;                                                \
                map->vals[probe_position] = val;                                                \
                map->buckets_used++;                                                            \
                if (shuffled)                                                                   \
                {                                                                               \
                    *shuffled = false;                                                          \
                }                                                                               \
                return NO_ERROR;                                                                \
            }                                                                                   \
            probe_position++;                                                                   \
        }                                                                                       \
                                                                                                \
        if (probe_position >= map->n_buckets || psl_curr >= map->max_psl)                       \
        {                                                                                       \
            if (grew)                                                                           \
            {                                                                                   \
                *grew = true;                                                                   \
            }                                                                                   \
            grow_map_##name(map);                                                               \
            h_u64 target_idx = compute_index_##name(map, &key);                                 \
            h_bool grew_twice = false;                                                          \
            h_result res = probe_##name(map, target_idx, key, val, 0, 0, &grew_twice);          \
            H_ASSERT(grew_twice == false, "h_map shouldn't be growing twice");                  \
            return res;                                                                         \
        }                                                                                       \
        return ret;                                                                             \
    }                                                                                           \
    static h_result h_put_##name(h_map_##name *map, key_type key, val_type val)                 \
    {                                                                                           \
        if (map->buckets_used >= map->n_buckets)                                                \
        {                                                                                       \
            grow_map_##name(map);                                                               \
            map->max_psl = max_psl(map->n_buckets);                                             \
        }                                                                                       \
        h_u64 index = compute_index_##name(map, &key);                                          \
        h_result probe_result = UNKNOWN_ERROR;                                                  \
        probe_result = probe_##name(map, index, key, val);                                      \
        return probe_result;                                                                    \
    }                                                                                           \
                                                                                                \
    static inline h_result hashmap_insert_##name(h_map_##name *map, key_type key, val_type val) \
    {                                                                                           \
        return h_put_##name(map, key, val);                                                     \
    }                                                                                           \
                                                                                                \
    static h_result h_retrieve_##name(h_map_##name *map, key_type key, val_type *o_val)         \
    {                                                                                           \
        h_u64 index = compute_index_##name(map, &key);                                          \
        h_u32 max_psl_dist = max_psl(map->n_buckets);                                           \
        for (h_u32 i = index; i < map->n_buckets && i < max_psl_dist + index; i++)              \
        {                                                                                       \
            if (map->fulls[i])                                                                  \
            {                                                                                   \
                if (map->psls[i] > i)                                                           \
                {                                                                               \
                    o_val = NULL;                                                               \
                    return FOUND_HIGHER_PSL;                                                    \
                }                                                                               \
                else                                                                            \
                {                                                                               \
                    if (map->equal_func(&map->keys[i], &key))                                   \
                    {                                                                           \
                        o_val = &map->vals[i];                                                  \
                        return NO_ERROR;                                                        \
                    }                                                                           \
                }                                                                               \
            }                                                                                   \
            else                                                                                \
            {                                                                                   \
                o_val = NULL;                                                                   \
                return EMPTY_BUCKET;                                                            \
            }                                                                                   \
        }                                                                                       \
        return EXCEEDED_MAP_BOUNDS;                                                             \
    }                                                                                           \
                                                                                                \
    static inline h_bool h_free_##name(h_map_##name *map)                                       \
    {                                                                                           \
        if (map->fulls)                                                                         \
        {                                                                                       \
            free(map->fulls);                                                                   \
            free(map->psls);                                                                    \
            free(map->keys);                                                                    \
            free(map->vals);                                                                    \
        }                                                                                       \
        else                                                                                    \
        {                                                                                       \
            return H_FAILURE;                                                                   \
        }                                                                                       \
        return H_SUCCESS;                                                                       \
    }

#endif