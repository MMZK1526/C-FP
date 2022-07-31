#include <iso646.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../mmzklist.h"

// Print out an int list.
static void dump_int_list(mmzk_list_t *list) {
  mmzk_list_iterator_t iter = mmzk_list_iterator(list);
  while (mmzk_list_has_next(iter)) {
    printf("%d --> ", *(int32_t *)mmzk_list_yield(&iter));
  }

  printf("nil");
}

static bool int_eq(const void *i1, const void *i2) {
  return *(int32_t *)i1 == *(int32_t *)i2;
}

static void *int_copy(const void *i1) {
  int32_t *result = malloc(sizeof(int32_t));
  *result = *(int32_t *)i1;
  return result;
}

static void int_free(void *i1) {
  free(i1);
}

static bool list_eq(const void *l1, const void *l2) {
  return mmzk_list_equal((mmzk_list_t *)l1, (mmzk_list_t *)l2);
}

static void *list_copy(const void *l1) {
  return mmzk_list_copy((mmzk_list_t *)l1);
}

static void list_free(void *l1) {
  mmzk_list_free((mmzk_list_t *)l1);
}

static mmzk_funs_t int_funs = (mmzk_funs_t){&int_eq, &int_copy, &int_free};

static mmzk_funs_t list_funs = (mmzk_funs_t){&list_eq, &list_copy, &list_free};

// Helper function for mmzk_list_map.
static void *cons_worker(const void *list, void *elem) {
  return mmzk_list_cons((const void *)elem, (mmzk_list_t *)list);
}

// Compute the power set of the list [1..n], returning a list of lists of ints.
static mmzk_list_t *power(int32_t n) {
  mmzk_list_t *result;

  assert (n >= 0);

  if (n == 0) { // Base case: [[]]
    mmzk_list_t *nil = mmzk_list_new(int_funs); // nil :: [Int]; nil := []
    result = mmzk_list_new(list_funs); // result :: [[a]]; result := []
    mmzk_list_set_persistence(result, false);
    result = mmzk_list_cons((const void *)nil, result); // result := nil : result
    mmzk_list_free(nil);
    return result;
  } else { // Inductive case: power (n - 1) ++ map (n :) (power (n - 1))
    int32_t *_n = malloc(sizeof(int32_t));
    *_n = n;
    mmzk_list_t *result = power(n - 1); // result := power (n - 1)
    mmzk_list_t *temp = mmzk_list_copy(result); // temp := result
    // result := temp ++ map (n :) result
    result = mmzk_list_concat(temp, mmzk_list_map(list_funs, cons_worker, result, _n));
    int_free(_n);
    return result;
  }
}

int32_t main(int32_t argc, char **argv) {
  mmzk_list_t *result = power(5);
  mmzk_list_iterator_t iter = mmzk_list_iterator(result);
  while (mmzk_list_has_next(iter)) {
    dump_int_list((mmzk_list_t *)mmzk_list_yield(&iter));
    puts("");
  }
  mmzk_list_free(result);
  return 0;
}
