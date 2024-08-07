/*****************************************************************************/
/*  Math Spatial Engine - Open source 2D geometry algorithm library          */
/*                                                                           */
/*  Copyright (C) 2013-2024 Merlot.Rain                                      */
/*                                                                           */
/*  This library is free software, licensed under the terms of the GNU       */
/*  General Public License as published by the Free Software Foundation,     */
/*  either version 3 of the License, or (at your option) any later version.  */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*****************************************************************************/


#ifdef HASHMAP_TEST

static size_t
deepcount (struct hashmap *map)
{
  size_t count = 0;
  for (size_t i = 0; i < map->nbuckets; i++)
    {
      if (bucket_at (map, i)->dib)
        {
          count++;
        }
    }
  return count;
}

#  ifdef __GNUC__
#    pragma GCC diagnostic ignored "-Wpedantic"
#  endif
#  ifdef __clang__
#    pragma GCC diagnostic ignored "-Wunknown-warning-option"
#    pragma GCC diagnostic ignored "-Wcompound-token-split-by-macro"
#    pragma GCC diagnostic ignored                                            \
        "-Wgnu-statement-expression-from-macro-expansion"
#  endif
#  ifdef __GNUC__
#    pragma GCC diagnostic ignored "-Wunused-parameter"
#  endif

#  include <stdlib.h>
#  include <string.h>
#  include <time.h>
#  include <assert.h>
#  include <stdio.h>
#  include "hashmap.h"

static bool rand_alloc_fail = false;
static int rand_alloc_fail_odds = 3; // 1 in 3 chance malloc will fail.
static uintptr_t total_allocs = 0;
static uintptr_t total_mem = 0;

static void *
xmalloc (size_t size)
{
  if (rand_alloc_fail && rand () % rand_alloc_fail_odds == 0)
    {
      return NULL;
    }
  void *mem = malloc (sizeof (uintptr_t) + size);
  assert (mem);
  *(uintptr_t *)mem = size;
  total_allocs++;
  total_mem += size;
  return (char *)mem + sizeof (uintptr_t);
}

static void
xfree (void *ptr)
{
  if (ptr)
    {
      total_mem -= *(uintptr_t *)((char *)ptr - sizeof (uintptr_t));
      free ((char *)ptr - sizeof (uintptr_t));
      total_allocs--;
    }
}

static void
shuffle (void *array, size_t numels, size_t elsize)
{
  char tmp[elsize];
  char *arr = array;
  for (size_t i = 0; i < numels - 1; i++)
    {
      int j = i + rand () / (RAND_MAX / (numels - i) + 1);
      memcpy (tmp, arr + j * elsize, elsize);
      memcpy (arr + j * elsize, arr + i * elsize, elsize);
      memcpy (arr + i * elsize, tmp, elsize);
    }
}

static bool
iter_ints (const void *item, void *udata)
{
  int *vals = *(int **)udata;
  vals[*(int *)item] = 1;
  return true;
}

static int
compare_ints_udata (const void *a, const void *b, void *udata)
{
  return *(int *)a - *(int *)b;
}

static int
compare_strs (const void *a, const void *b, void *udata)
{
  return strcmp (*(char **)a, *(char **)b);
}

static uint64_t
hash_int (const void *item, uint64_t seed0, uint64_t seed1)
{
  return hashmap_xxhash3 (item, sizeof (int), seed0, seed1);
  // return hashmap_sip(item, sizeof(int), seed0, seed1);
  // return hashmap_murmur(item, sizeof(int), seed0, seed1);
}

static uint64_t
hash_str (const void *item, uint64_t seed0, uint64_t seed1)
{
  return hashmap_xxhash3 (*(char **)item, strlen (*(char **)item), seed0,
                          seed1);
  // return hashmap_sip(*(char**)item, strlen(*(char**)item), seed0, seed1);
  // return hashmap_murmur(*(char**)item, strlen(*(char**)item), seed0, seed1);
}

static void
free_str (void *item)
{
  xfree (*(char **)item);
}

static void
all (void)
{
  int seed = getenv ("SEED") ? atoi (getenv ("SEED")) : time (NULL);
  int N = getenv ("N") ? atoi (getenv ("N")) : 2000;
  printf ("seed=%d, count=%d, item_size=%zu\n", seed, N, sizeof (int));
  srand (seed);

  rand_alloc_fail = true;

  // test sip and murmur hashes
  assert (hashmap_sip ("hello", 5, 1, 2) == 2957200328589801622);
  assert (hashmap_murmur ("hello", 5, 1, 2) == 1682575153221130884);
  assert (hashmap_xxhash3 ("hello", 5, 1, 2) == 2584346877953614258);

  int *vals;
  while (!(vals = xmalloc (N * sizeof (int))))
    {
    }
  for (int i = 0; i < N; i++)
    {
      vals[i] = i;
    }

  struct hashmap *map;

  while (!(map = hashmap_new (sizeof (int), 0, seed, seed, hash_int,
                              compare_ints_udata, NULL, NULL)))
    {
    }
  shuffle (vals, N, sizeof (int));
  for (int i = 0; i < N; i++)
    {
      // // printf("== %d ==\n", vals[i]);
      assert (map->count == (size_t)i);
      assert (map->count == hashmap_count (map));
      assert (map->count == deepcount (map));
      const int *v;
      assert (!hashmap_get (map, &vals[i]));
      assert (!hashmap_delete (map, &vals[i]));
      while (true)
        {
          assert (!hashmap_set (map, &vals[i]));
          if (!hashmap_oom (map))
            {
              break;
            }
        }

      for (int j = 0; j < i; j++)
        {
          v = hashmap_get (map, &vals[j]);
          assert (v && *v == vals[j]);
        }
      while (true)
        {
          v = hashmap_set (map, &vals[i]);
          if (!v)
            {
              assert (hashmap_oom (map));
              continue;
            }
          else
            {
              assert (!hashmap_oom (map));
              assert (v && *v == vals[i]);
              break;
            }
        }
      v = hashmap_get (map, &vals[i]);
      assert (v && *v == vals[i]);
      v = hashmap_delete (map, &vals[i]);
      assert (v && *v == vals[i]);
      assert (!hashmap_get (map, &vals[i]));
      assert (!hashmap_delete (map, &vals[i]));
      assert (!hashmap_set (map, &vals[i]));
      assert (map->count == (size_t)(i + 1));
      assert (map->count == hashmap_count (map));
      assert (map->count == deepcount (map));
    }

  int *vals2;
  while (!(vals2 = xmalloc (N * sizeof (int))))
    {
    }
  memset (vals2, 0, N * sizeof (int));
  assert (hashmap_scan (map, iter_ints, &vals2));

  // Test hashmap_iter. This does the same as hashmap_scan above.
  size_t iter = 0;
  void *iter_val;
  while (hashmap_iter (map, &iter, &iter_val))
    {
      assert (iter_ints (iter_val, &vals2));
    }
  for (int i = 0; i < N; i++)
    {
      assert (vals2[i] == 1);
    }
  xfree (vals2);

  shuffle (vals, N, sizeof (int));
  for (int i = 0; i < N; i++)
    {
      const int *v;
      v = hashmap_delete (map, &vals[i]);
      assert (v && *v == vals[i]);
      assert (!hashmap_get (map, &vals[i]));
      assert (map->count == (size_t)(N - i - 1));
      assert (map->count == hashmap_count (map));
      assert (map->count == deepcount (map));
      for (int j = N - 1; j > i; j--)
        {
          v = hashmap_get (map, &vals[j]);
          assert (v && *v == vals[j]);
        }
    }

  for (int i = 0; i < N; i++)
    {
      while (true)
        {
          assert (!hashmap_set (map, &vals[i]));
          if (!hashmap_oom (map))
            {
              break;
            }
        }
    }

  assert (map->count != 0);
  size_t prev_cap = map->cap;
  hashmap_clear (map, true);
  assert (prev_cap < map->cap);
  assert (map->count == 0);

  for (int i = 0; i < N; i++)
    {
      while (true)
        {
          assert (!hashmap_set (map, &vals[i]));
          if (!hashmap_oom (map))
            {
              break;
            }
        }
    }

  prev_cap = map->cap;
  hashmap_clear (map, false);
  assert (prev_cap == map->cap);

  hashmap_free (map);

  xfree (vals);

  while (!(map = hashmap_new (sizeof (char *), 0, seed, seed, hash_str,
                              compare_strs, free_str, NULL)))
    ;

  for (int i = 0; i < N; i++)
    {
      char *str;
      while (!(str = xmalloc (16)))
        ;
      snprintf (str, 16, "s%i", i);
      while (!hashmap_set (map, &str))
        ;
    }

  hashmap_clear (map, false);
  assert (hashmap_count (map) == 0);

  for (int i = 0; i < N; i++)
    {
      char *str;
      while (!(str = xmalloc (16)))
        ;
      snprintf (str, 16, "s%i", i);
      while (!hashmap_set (map, &str))
        ;
    }

  hashmap_free (map);

  if (total_allocs != 0)
    {
      fprintf (stderr, "total_allocs: expected 0, got %lu\n", total_allocs);
      exit (1);
    }
}

#  define bench(name, N, code)                                                \
    { { if (strlen (name) > 0){ printf ("%-14s ", name);                      \
    }                                                                         \
    size_t tmem = total_mem;                                                  \
    size_t tallocs = total_allocs;                                            \
    uint64_t bytes = 0;                                                       \
    clock_t begin = clock ();                                                 \
    for (int i = 0; i < N; i++)                                               \
      {                                                                       \
        (code);                                                               \
      }                                                                       \
    clock_t end = clock ();                                                   \
    double elapsed_secs = (double)(end - begin) / CLOCKS_PER_SEC;             \
    double bytes_sec = (double)bytes / elapsed_secs;                          \
    printf ("%d ops in %.3f secs, %.0f ns/op, %.0f op/sec", N, elapsed_secs,  \
            elapsed_secs / (double)N * 1e9, (double)N / elapsed_secs);        \
    if (bytes > 0)                                                            \
      {                                                                       \
        printf (", %.1f GB/sec", bytes_sec / 1024 / 1024 / 1024);             \
      }                                                                       \
    if (total_mem > tmem)                                                     \
      {                                                                       \
        size_t used_mem = total_mem - tmem;                                   \
        printf (", %.2f bytes/op", (double)used_mem / N);                     \
      }                                                                       \
    if (total_allocs > tallocs)                                               \
      {                                                                       \
        size_t used_allocs = total_allocs - tallocs;                          \
        printf (", %.2f allocs/op", (double)used_allocs / N);                 \
      }                                                                       \
    printf ("\n");                                                            \
    }                                                                         \
    }

static void
benchmarks (void)
{
  int seed = getenv ("SEED") ? atoi (getenv ("SEED")) : time (NULL);
  int N = getenv ("N") ? atoi (getenv ("N")) : 5000000;
  printf ("seed=%d, count=%d, item_size=%zu\n", seed, N, sizeof (int));
  srand (seed);

  int *vals = xmalloc (N * sizeof (int));
  for (int i = 0; i < N; i++)
    {
      vals[i] = i;
    }

  shuffle (vals, N, sizeof (int));

  struct hashmap *map;
  shuffle (vals, N, sizeof (int));

  map = hashmap_new (sizeof (int), 0, seed, seed, hash_int, compare_ints_udata,
                     NULL, NULL);
  bench ("set", N, {
    const int *v = hashmap_set (map, &vals[i]);
    assert (!v);
  }) shuffle (vals, N, sizeof (int));
  bench ("get", N, {
    const int *v = hashmap_get (map, &vals[i]);
    assert (v && *v == vals[i]);
  }) shuffle (vals, N, sizeof (int));
  bench ("delete", N, {
    const int *v = hashmap_delete (map, &vals[i]);
    assert (v && *v == vals[i]);
  }) hashmap_free (map);

  map = hashmap_new (sizeof (int), N, seed, seed, hash_int, compare_ints_udata,
                     NULL, NULL);
  bench ("set (cap)", N, {
    const int *v = hashmap_set (map, &vals[i]);
    assert (!v);
  }) shuffle (vals, N, sizeof (int));
  bench ("get (cap)", N, {
    const int *v = hashmap_get (map, &vals[i]);
    assert (v && *v == vals[i]);
  }) shuffle (vals, N, sizeof (int));
  bench ("delete (cap)", N,
         {
           const int *v = hashmap_delete (map, &vals[i]);
           assert (v && *v == vals[i]);
         })

      hashmap_free (map);

  xfree (vals);

  if (total_allocs != 0)
    {
      fprintf (stderr, "total_allocs: expected 0, got %lu\n", total_allocs);
      exit (1);
    }
}

int
main (void)
{
  hashmap_set_allocator (xmalloc, xfree);

  if (getenv ("BENCH"))
    {
      printf ("Running hashmap.c benchmarks...\n");
      benchmarks ();
    }
  else
    {
      printf ("Running hashmap.c tests...\n");
      all ();
      printf ("PASSED\n");
    }
}

#endif