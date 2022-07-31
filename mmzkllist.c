#include <iso646.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmzkllist.h"


/* Helpers */

struct array_len {
  size_t len;
  const void **elems;
};

struct pure_gen {
  const void *cur;
  mmzk_pure_gen_t *generator;
};

mmzk_lframe_t array_gen(const void *ptr) {
  struct array_len *array_len = (struct array_len *)ptr;
  if (array_len->len == 0) {
    free(ptr);
    return (mmzk_lframe_t){ .generator = NULL };
  }

  const void *elem = array_len->elems[0];
  array_len->elems = array_len->elems + 1;
  array_len->len--;

  return (mmzk_lframe_t){ .arg = array_len, .result = elem, .generator = array_gen };
}

mmzk_lframe_t generator_gen(const void *ptr) {
  struct pure_gen *pure_gen = (struct pure_gen *)ptr;
  const void *elem = (pure_gen->generator)(pure_gen->cur);
  if (elem == NULL) {
    free(ptr);
    return (mmzk_lframe_t){ .generator = NULL };
  }

  pure_gen->cur = elem;

  return (mmzk_lframe_t){ .arg = pure_gen, .result = elem, .generator = generator_gen };
}


/* Definitions */

typedef struct node {
  unsigned int prev_count;
  const void *elem;
  struct node *next;
  mmzk_lframe_gen_t *generator;
  const void *arg;
} node_t;

struct mmzk_llist {
  mmzk_funs_t funs;
  node_t *node;
};


/* Construction & Destruction */

mmzk_llist_t *mmzk_llist_new(mmzk_funs_t funs) {
  mmzk_llist_t *list = malloc(sizeof(mmzk_llist_t));
  list->funs = funs;
  list->node = malloc(sizeof(node_t));
  list->node->prev_count = 0;
  list->node->next = NULL;
  list->node->generator = NULL;

  return list;
}

mmzk_llist_t *mmzk_llist_from_array(mmzk_funs_t funs, size_t len, const void *elems[]) {
  mmzk_llist_t *list = malloc(sizeof(mmzk_llist_t));
  list->funs = funs;
  list->node = malloc(sizeof(node_t));
  list->node->prev_count = 0;
  struct array_len *array_len = malloc(sizeof(struct array_len));
  array_len->len = len;
  array_len->elems = elems;
  list->node->arg = array_len;
  list->node->generator = array_gen;

  return list;
}

mmzk_llist_t *mmzk_llist_from_generator(mmzk_funs_t funs, mmzk_pure_gen_t *generator, const void *seed) {
  mmzk_llist_t *list = malloc(sizeof(mmzk_llist_t));
  list->funs = funs;
  list->node = malloc(sizeof(node_t));
  struct pure_gen *pure_gen = malloc(sizeof(struct pure_gen));
  pure_gen->generator = generator;
  pure_gen->cur = seed;
  list->node->prev_count = 0;
  list->node->arg = pure_gen;
  list->node->generator = generator_gen;

  return list;
}






bool int_eq(const void *i1, const void *i2) {
  return *(int *)i1 == *(int *)i2;
}

void *int_copy(const void *i1) {
  int *result = malloc(sizeof(int));
  *result = *(int *)i1;
  return result;
}

void int_free(const void *i1) {
  free(i1);
}

void dump_int_list(mmzk_llist_t *list) {
  node_t *node = list->node;
  
  while (node->generator != NULL || node->next != NULL) {
    const void *elem;
    if (node->generator == NULL) {
      elem = node->elem;
    } else {
      mmzk_lframe_t frame = (node->generator)(node->arg);
      if (frame.generator == NULL) {
        break;
      }
      node->elem = (list->funs.copy_fun)(frame.result);
      node->generator = NULL;
      node->arg = NULL;
      node->next = malloc(sizeof(node_t));
      node->next->prev_count = 0;
      node->next->arg = frame.arg;
      node->next->generator = frame.generator;
    }
    printf("%d --> ", *(int *)(node->elem));
    node = node->next;
  }
  printf("nil");
}

int from_array_test() {
  int *i1 = malloc(sizeof(int));
  int *i2 = malloc(sizeof(int));
  int *i3 = malloc(sizeof(int));
  *i1 = 1;
  *i2 = 2;
  *i3 = 3;

  mmzk_llist_t *l123 = mmzk_llist_from_array((mmzk_funs_t){&int_eq, &int_copy, &int_free}, 3, (const void *[3]){i1, i2, i3});

  printf("l123 content: ");
  dump_int_list(l123);
  puts("");

  int_free(i1);
  int_free(i2);
  int_free(i3);

  // mmzk_llist_free(l123);

  return EXIT_SUCCESS;
}

const void *succ(const void *i1) {
  int *result = malloc(sizeof(int *));
  if (i1 != NULL) {
    *result = 1 + *(int *)i1;
  } else {
    *result = 0;
  }
  return result;
}

int main(int argc, char **argv) {
  // from_array_test();
  mmzk_llist_t *nats = mmzk_llist_from_generator((mmzk_funs_t){&int_eq, &int_copy, &int_free}, succ, NULL);
  dump_int_list(nats);

  return EXIT_SUCCESS;
}
