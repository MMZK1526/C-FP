#ifndef MMZK1526
#define MMZK1526
#endif /* MMZK1526 */

#ifndef MMZK_LIST_BASE_H
#define MMZK_LIST_BASE_H

#include <assert.h>
#include <stdint.h>

#ifndef MMZKTYPEDEF_EQUAL_FUN
#define MMZKTYPEDEF_EQUAL_FUN
typedef bool mmzk_eq_fun(const void *, const void *);
#endif /* MMZKTYPEDEF_EQUAL_FUN */

#ifndef MMZKTYPEDEF_COPY_FUN
#define MMZKTYPEDEF_COPY_FUN
typedef void *mmzk_copy_fun(const void *);
#endif /* MMZKTYPEDEF_COPY_FUN */

#ifndef MMZKTYPEDEF_FREE_FUN
#define MMZKTYPEDEF_FREE_FUN
typedef void mmzk_free_fun(void *);
#endif /* MMZKTYPEDEF_FREE_FUN */

#define UNREACHABLE(X) (assert(false), X);

// Necessary functions for a list.
//
// EQ_FUN determines structural equality for the elements;
// COPY_FUN is called to create copies of elements;
// FREE_FUN is called to free elements when the node they live in is freed.
//
// If COPY_FUN always creates a new instance, then FREE_FUN must always free this instance. In this case, we can modify
// the original element arbitrarily without affecting the element in the list.
//
// However, one can manage a counter within the element, each "copy" would increment the counter; each "free" decrements
// it and only reclaim the memory if the counter is zero. This is useful if the elements are very large and deep copying
// is expensive. In this case, we should not modify the original element since it is not truly copied and such
// modification would affect the corresponding element within the list.
//
// For the complexity analysis in this module, it is assumed that all these functions have constant time complexity.
typedef struct mmzk_funs {
  mmzk_eq_fun *eq_fun;
  mmzk_copy_fun *copy_fun;
  mmzk_free_fun *free_fun;
} mmzk_funs_t;

// A predicate type.
typedef bool predicate_t(const void *);

// A persistent and functional list structure similar to Haskell's [], but it is strict.
// The elements are of type const void * and they should not be modified.
typedef struct mmzk_list mmzk_list_t;

// Tuple of strict lists.
typedef struct mmzk_list_tuple {
  mmzk_list_t *fst;
  mmzk_list_t *snd;
} mmzk_list_tuple_t;

// Iterator for the strict list type.
typedef struct mmzk_list_iterator {
  size_t length;
  struct node *node;
} mmzk_list_iterator_t;

// A persistent and functional lazy list structure similar to Haskell's [].
// The elements are of type const void * and they should not be modified.
typedef struct mmzk_llist mmzk_llist_t;

// Tuple of lazy lists.
typedef struct mmzk_llist_tuple {
  mmzk_llist_t *fst;
  mmzk_llist_t *snd;
} mmzk_llist_tuple_t;

typedef struct mmzk_lframe {
  const void *result;
  const void *arg;
  struct mmzk_lframe (*generator)(const void *);
} mmzk_lframe_t;

typedef struct mmzk_lframe mmzk_lframe_gen_t(const void *);
typedef const void *mmzk_pure_gen_t(const void *);

#endif /* MMZK_LIST_BASE_H */
