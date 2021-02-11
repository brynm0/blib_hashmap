

#include "hashmap.h"
#include "string.h"
#include <chrono>
#include "blib_utils.h"
#include "len_string.h"
#include "stdlib.h"
#include "debug_file_io.h"
#include <unordered_map>

static inline std::chrono::steady_clock::time_point current_time()
{
    return std::chrono::steady_clock::now();
}

static inline std::chrono::microseconds microseconds_elapsed(std::chrono::steady_clock::time_point start,
                                                             std::chrono::steady_clock::time_point end)
{
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration;
}

// HASH_FUNCTION(str_hash)
// {
//     len_string *str = (len_string *)to_hash;
//     size_t hash = 5381;
//     int c;
//     char *cpy = str->str;
//     while (*cpy && (c = *cpy++))
//     {
//         hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
//     }
//     // fprintf(stderr, "%s hash: %llu\n", str, hash);
//     return hash;
// }

// HASH_FUNCTION(int_hash)
// {
//     int *i_hash = (int *)to_hash;
//     return (h_u64)(*i_hash);
// }

HASH_FUNCTION(u32_hash, u32)
{
    return (h_u64)*to_hash;
}

// HASH_EQUALS(str_equals)
// {
//     len_string *a_str = (len_string *)a;
//     len_string *b_str = (len_string *)b;
//     return (strcmp(a_str->str, b_str->str) == 0);
// }

// HASH_EQUALS(int_equals)
// {
//     return *(int *)a == *(int *)b;
// }

HASH_EQUALS(u32_equals, u32)
{
    return *a == *b;
}

HASHMAP_INIT(u32_len_string, u32, len_string, u32_hash, u32_equals);

// // HASH_MAP_TYPE_INIT(string_int, char *, int, str_hash, str_equals);
// HASHMAP_INSERT_FUNC(u32_len_string, u32, len_string);

static void run_tests(u32 *keys, len_string *vals, u32 count)
{

    h_map_u32_len_string h = h_init_u32_len_string();
    u32 actual_num_inputs = 0;

    for (int i = 0; i < count; i++)
    {
        // if (i % 1000 == 0)
        // {
        //     fprintf(stderr, "%d / %d\r", i, count);
        // }
        h_result ins_result = hashmap_insert_u32_len_string(&h, keys[i], vals[i]);
        if (ins_result == NO_ERROR)
        {
            actual_num_inputs++;
        }
    }
    h_free_u32_len_string(&h);
    // auto end = current_time();
    // auto us_elapsed = microseconds_elapsed(start, end);
    // fprintf(stderr, "Occupancy: %.2f%\n", 100.0f * ((float)h.buckets_used / (float)h.n_buckets));
    // fprintf(stderr, "n_buckets: %d / %d \nn_inputs: %d \nMemory: %d bytes\nTime: %.2fus\n\n",
    //         h.buckets_used, h.n_buckets,
    //         actual_num_inputs,
    //         malloc_ctr,
    //         (float)us_elapsed.count());
    // fflush(stderr);
}

static void run_unordered_map_tests(u32 *keys, len_string *vals, u32 count)
{
    std::unordered_map<u32, len_string> h;
    u32 actual_num_inputs = 0;
    for (int i = 0; i < count; i++)
    {

        // if (i % 1000 == 0)
        // {
        //     fprintf(stderr, "%d / %d\r", i, count);
        // }
        u32 key = keys[i];
        len_string val = vals[i]; //l_string(buf);
        if (h.find(key) == h.end())
        {
            h[key] = val;
            actual_num_inputs++;
        }
    }

    // auto end = current_time();
    // auto us_elapsed = microseconds_elapsed(start, end);
    // fprintf(stderr, "unordered_map:\nMemory: %d elements\nTime: %.2fus\n\n",
    //         h.size(), (float)us_elapsed.count());
    // fflush(stderr);
}

// len_string print_bucket(const h_map &map, const bucket &bucket)
// {
//     len_string l = l_string("");
//     char buf[256];
//     sprintf(buf, "h_bool full = %d\n h_u32 psl = %ld\n void* key = %lld\n void* val = %lld\n", bucket.full, bucket.psl, (h_u64)bucket.key, (h_u64)bucket.val);
//     append_to_len_string(&l, buf);

//     return l;
// }

// static void run_occupancy_benchmark(h_u32 tests_x, h_u32 tests_y)
// {
//     len_string occupancy_vals = l_string("");
//     for (int i = 0; i < tests_x; i++)
//     {
//         h_map h = hashmap(sizeof(u32), sizeof(len_string), u32_hash, u32_equals, tests_y);
//         auto t = current_time();

//         srand(t.time_since_epoch().count());
//         for (int j = 0; j < tests_y; j++)
//         {
//             u32 key = (u32)rand();
//             char buf[64];
//             itoa(j, buf, 10);
//             len_string val = l_string(buf);
//             h_result result = hashmap_insert_u32_len_string(&h, &key, &val);
//             if (result != NO_ERROR)
//             {
//                 break;
//             }
//         }
//         char buf[32];
//         sprintf(buf, "%.5f", ((float)h.buckets_used / (float)h.n_buckets));
//         append_to_len_string(&occupancy_vals, buf);
//         append_to_len_string(&occupancy_vals, ",");

//         // fprintf(stderr, "Occupancy: %.2f % (%d / %d)\n\n", 100.0f * ((float)h.buckets_used / (float)h.n_buckets), h.buckets_used, h.n_buckets);
//         // for (int i = 0; i < h.n_buckets; i++)
//         // {
//         //     len_string l = print_bucket(h, h.buckets[i]);
//         //     fprintf(stderr, l.str);
//         //     fprintf(stderr, "\n");
//         //     free_l_string(&l);
//         // }
//     }
//     char file_name[64];
//     sprintf(file_name, "W:\\blib_hashmap\\results\\occupancy_%d.txt", tests_y);
//     FILE *f = fopen(file_name, "wb");
//     fwrite(occupancy_vals.str, sizeof(u8), occupancy_vals.string_len, f);
//     fclose(f);
// }

int main(int argc, char **argv)
{

    printf("Building test buffer...\n");
    u32 test_count = 25000000; // (u32)pow(2, 16);
    u32 *keys = (u32 *)malloc(sizeof(u32) * test_count);
    len_string *vals = (len_string *)malloc(sizeof(len_string) * test_count);
    for (int i = 0; i < test_count; i++)
    {
        srand(1);
        keys[i] = (u32)rand();
        char buf[64];
        itoa(i, buf, 10);
        vals[i] = l_string(buf);
    }
    printf("Beginning tests...\n");
    u32 num_test_iter = 128;
    if (argc > 1)
    {
        num_test_iter = atoi(argv[1]);
    }
    printf("num_test_iter: %d\n", num_test_iter);
    auto start = current_time();
    for (int i = 0; i < num_test_iter; i++)
    {
        // fprintf(stdout, "h_map: %d / %d\r", i, num_test_iter);
        h_map_u32_len_string h = h_init_u32_len_string();
        u32 actual_num_inputs = 0;

        fprintf(stdout, "%d / %d\r", i, num_test_iter);

        for (int i = 0; i < test_count; i++)
        {

            h_result ins_result = hashmap_insert_u32_len_string(&h, keys[i], vals[i]);
            if (ins_result == NO_ERROR)
            {
                actual_num_inputs++;
            }
        }
        h_free_u32_len_string(&h);
    }
    auto end = current_time();
    auto us_elapsed = microseconds_elapsed(start, end);
    fprintf(stdout, "h_map time: %.3fs\n", (float)us_elapsed.count() / 1000000.0f);

    // start = current_time();
    // for (int i = 0; i < num_test_iter; i++)
    // {
    //     fprintf(stdout, "std::unordered_map: %d / %d\r", i, num_test_iter);
    //     run_unordered_map_tests(keys, vals, test_count);
    // }
    // end = current_time();
    // us_elapsed = microseconds_elapsed(start, end);
    // fprintf(stdout, "std::unordered_map time: %.3fs\n", (float)us_elapsed.count() / 1000000.0f);

    // run_occupancy_benchmark(N_TESTS_X, N_TESTS_Y);
    return 0;
}