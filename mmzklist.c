#include <assert.h>
#include <iso646.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmzklist.h"


/* Definitions */

struct node {
  unsigned int prev_count;
  const void *elem;
  struct node *next;
};

struct mmzk_list {
  bool is_persistent;
  mmzk_funs_t funs;
  struct node *node;
  size_t length;
};


/* Helpers */

#define INIT_LIST(FUNS, PERSISTENCE, LIST) do {\
  LIST->funs = FUNS;\
  LIST->node = NULL;\
  LIST->is_persistent = PERSISTENCE;\
  LIST->length = 0;\
} while (false)

static void *_fold(size_t len, void *(*worker)(const void *, void *), void *accum, struct node *node) {
  if (len == 0) {
    return accum;
  }

  return worker(node->elem, _fold(len - 1, worker, accum, node->next));
}


/* Construction & Destruction */

mmzk_list_t *mmzk_list_new(mmzk_funs_t funs) {
  mmzk_list_t *list = malloc(sizeof(mmzk_list_t));
  INIT_LIST(funs, true, list);

  return list;
}

mmzk_list_t *mmzk_list_from_array(mmzk_funs_t funs, size_t len, void *elems[]) {
  mmzk_list_t *list = malloc(sizeof(mmzk_list_t));
  INIT_LIST(funs, true, list);
  list->length = len;

  for (size_t i = len; i > 0; i--) {
    struct node *node = malloc(sizeof(struct node));
    node->elem = (list->funs.copy_fun)(elems[i - 1]);
    node->next = list->node;
    node->prev_count = 0;
    list->node = node;
  }

  return list;
}

void **mmzk_list_to_array(mmzk_list_t *list, mmzk_funs_t *funs, size_t *len) {
  void **result = malloc(list->length * sizeof(void *));
  struct node *node = list->node;

  for (size_t i = 0; i < list->length; (node = node->next, i++)) {
    result[i] = (list->funs.copy_fun)(node->elem);
  }

  if (funs != NULL) {
    *funs = list->funs;
  }

  if (len != NULL) {
    *len = list->length;
  }

  if (!list->is_persistent) {
    mmzk_list_free(list);
  }

  return result;
}

void mmzk_list_free(mmzk_list_t *list) {
  struct node *node = list->node;

  if (node != NULL) {
    while (node != NULL) {
      if (node->prev_count > 0) {
        node->prev_count--;
        break;
      }
      struct node *temp = node;
      node = node->next;
      (list->funs.free_fun)((void *)(temp->elem));
      free(temp);
    }
  }

  free(list);
}

mmzk_list_t *mmzk_list_copy(mmzk_list_t *list) {
  mmzk_list_t *result = malloc(sizeof(mmzk_list_t));
  result->funs = list->funs;
  result->length = list->length;
  result->node = list->node;
  result->is_persistent = list->is_persistent;

  if (list->node != NULL) {
    list->node->prev_count++;
  }

  return result;
}

void mmzk_list_set_persistence(mmzk_list_t *list, bool persistence) {
  list->is_persistent = persistence;
}


/* Query */

size_t mmzk_list_length(mmzk_list_t *list) {
  return list->length;
}

void *mmzk_list_get(mmzk_list_t *list, size_t index) {
  struct node *node = list->node;

  if (index >= list->length) {
    return NULL;
  }

  while (node != NULL) {
    if (index == 0) {
      return (list->funs.copy_fun)(node->elem);
    }
    index--;
    node = node->next;
  }

  return UNREACHABLE(NULL);
}

void *mmzk_list_get_end(mmzk_list_t *list, size_t index) {
  struct node *slow = list->node;
  struct node *fast = list->node;
  index++;

  if (index >= list->length) {
    return NULL;
  }

  while (fast != NULL) {
    if (index == 0) {
      break;
    }
    index--;
    fast = fast->next;
  }

  while (fast != NULL) {
    fast = fast->next;
    slow = slow->next;
  }

  return (list->funs.copy_fun)(slow->elem);
}

bool mmzk_list_is_elem(const void *element, mmzk_list_t *list) {
  struct node *node = list->node;
  while (node != NULL) {
    if ((list->funs.eq_fun)(element, node->elem)) {
      return true;
    }
    node = node->next;
  }

  return false;
}

bool mmzk_list_equal(mmzk_list_t *list1, mmzk_list_t *list2) {
  if (list1->length != list2->length) {
    return false;
  }

  struct node *node1 = list1->node;
  struct node *node2 = list2->node;

  while (node1 != NULL) {
    assert(node2 != NULL);

    if (!(list1->funs.eq_fun)(node1->elem, node2->elem)) {
      return false;
    }

    node1 = node1->next;
    node2 = node2->next;
  }

  return true;
}


/* Composition */

mmzk_list_t *mmzk_list_cons(const void *elem, mmzk_list_t *list) {
  mmzk_list_t *result = malloc(sizeof(mmzk_list_t));
  result->funs = list->funs;
  result->length = list->length + 1;
  result->is_persistent = list->is_persistent;
  struct node *node = malloc(sizeof(struct node));
  node->elem = (list->funs.copy_fun)(elem);
  node->next = list->node;
  node->prev_count = 0;
  result->node = node;

  if (!list->is_persistent) {
    free(list);
  } else if (list->node != NULL) {
    list->node->prev_count++;
  }

  return result;
}

mmzk_list_t *mmzk_list_concat(mmzk_list_t *list1, mmzk_list_t *list2) {
  struct node *node1 = list1->node;
  struct node *node2 = list2->node;

  mmzk_list_t *result = malloc(sizeof(mmzk_list_t));
  result->is_persistent = list1->is_persistent || list2->is_persistent;
  result->funs = list1->funs;
  result->length = list1->length + list2->length;

  if (!list2->is_persistent) {
    free(list2);
  } else if (list2->node != NULL) {
    list2->node->prev_count++;
  }

  if (node1 == NULL) {
    result->node = node2;
  } else {
    struct node *node = malloc(sizeof(struct node));
    result->node = node;
    node->elem = (list1->funs.copy_fun)(node1->elem);
    node->prev_count = 0;
    struct node *cur = node;
    node1 = node1->next;
    while (node1 != NULL) {
      node = malloc(sizeof(struct node));
      node->elem = (list1->funs.copy_fun)(node1->elem);
      node->prev_count = 0;
      cur->next = node;
      cur = cur->next;
      node1 = node1->next;
    }
    cur->next = node2;
  }

  if (!list1->is_persistent) {
    mmzk_list_free(list1);
  }

  return result;
}


/* Decomposition */

void *mmzk_list_tail(mmzk_list_t *list) {
  if (list->length == 0) {
    if (!list->is_persistent) {
      mmzk_list_free(list);
    }
    return NULL;
  }

  struct node *node = list->node;

  mmzk_list_t *result = malloc(sizeof(mmzk_list_t));
  result->funs = list->funs;
  result->length = list->length - 1;
  result->is_persistent = list->is_persistent;
  result->node = node->next;
  if (result->node != NULL) {
    result->node->prev_count++;
  }

  if (!list->is_persistent) {
    mmzk_list_free(list);
  }

  return result;
}

void *mmzk_list_init(mmzk_list_t *list) {
  if (list->length == 0) {
    if (!list->is_persistent) {
      mmzk_list_free(list);
    }
    return NULL;
  }

  mmzk_list_t *result = malloc(sizeof(mmzk_list_t));
  struct node *node = list->node;
  result->is_persistent = list->is_persistent;
  result->funs = list->funs;
  result->length = list->length - 1;
  if (list->is_persistent) {
    node->prev_count++;
  } else {
    free(list);
  }
  result->node = node;

  return result;
}

void *mmzk_list_take(size_t i, mmzk_list_t *list) {
  mmzk_list_t *result = malloc(sizeof(mmzk_list_t));
  struct node *node = list->node;
  result->funs = list->funs;
  result->is_persistent = list->is_persistent;
  result->length = list->length > i ? i : list->length;
  result->node = node;

  if (!list->is_persistent) {
    free(list);
  } else if (list->node != NULL) {
    list->node->prev_count++;
  }

  return result;
}

void *mmzk_list_drop(size_t i, mmzk_list_t *list) {
  mmzk_list_t *result = malloc(sizeof(mmzk_list_t));
  INIT_LIST(list->funs, list->is_persistent, result);

  if (i >= list->length) {
    if (!list->is_persistent) {
      mmzk_list_free(list);
    }
    return result;
  }

  struct node *node = list->node;
  result->length = list->length - i;
  while (i != 0) {
    node = node->next;
    i--;
  }

  node->prev_count++;
  result->node = node;
  if (!list->is_persistent) {
    mmzk_list_free(list);
  }

  return result;
}

mmzk_list_tuple_t mmzk_list_split_at(size_t i, mmzk_list_t *list) {
  mmzk_list_t *result1 = malloc(sizeof(mmzk_list_t));
  mmzk_list_t *result2 = malloc(sizeof(mmzk_list_t));
  INIT_LIST(list->funs, list->is_persistent, result2);
  result1->is_persistent = list->is_persistent;
  struct node *node = list->node;

  result1->funs = list->funs;
  result1->node = node;
  if (node != NULL && list->is_persistent) {
    node->prev_count++;
  }

  if (i >= list->length) {
    result1->length = list->length;
    if (!list->is_persistent) {
      free(list);
    }
    return (mmzk_list_tuple_t) { .fst = result1, .snd = result2 };
  }

  result1->length = i;
  result2->length = list->length - i;
  while (i != 0) {
    node = node->next;
    i--;
  }
  node->prev_count++;
  result2->node = node;

  if (!list->is_persistent) {
    free(list);
  }

  return (mmzk_list_tuple_t) { .fst = result1, .snd = result2 };
}

mmzk_list_tuple_t mmzk_list_span(predicate_t *predicate, mmzk_list_t *list) {
  mmzk_list_t *result1 = malloc(sizeof(mmzk_list_t));
  mmzk_list_t *result2 = malloc(sizeof(mmzk_list_t));
  INIT_LIST(list->funs, list->is_persistent, result2);
  result1->is_persistent = list->is_persistent;
  struct node *node = list->node;
  size_t i = 0;

  result1->funs = list->funs;
  result1->node = node;
  if (node != NULL && list->is_persistent) {
    node->prev_count++;
  }

  while (node != NULL) {
    if(predicate(node->elem)) {
      i++;
      node = node->next;
    } else {
      break;
    }
  }

  result1->length = i;
  result2->length = list->length - i;
  result2->node = node;

  if (!list->is_persistent) {
    free(list);
  }

  return (mmzk_list_tuple_t) { .fst = result1, .snd = result2 };
}


/* Transformation */

mmzk_list_t *mmzk_list_map(mmzk_funs_t funs, void *(*worker)(const void *, void *), mmzk_list_t *list,
    void *arg) {
  mmzk_list_t *result;

  result = malloc(sizeof(mmzk_list_t));
  INIT_LIST(funs, list->is_persistent, result);
  result->length = list->length;
  struct node *dummy = malloc(sizeof(struct node));
  struct node *node1 = list->node;
  struct node *node = dummy;

  while (node1 != NULL) {
    node->next = malloc(sizeof(struct node));
    node->next->prev_count = 0;
    node->next->elem = worker(node1->elem, arg);
    node = node->next;
    node1 = node1->next;
  }
  node->next = NULL;
  result->node = dummy->next;
  free(dummy);

  if (!list->is_persistent) {
    mmzk_list_free(list);
  }

  return result;
}

mmzk_list_t *mmzk_list_filter(predicate_t *predicate, mmzk_list_t *list) {
  mmzk_list_t *result = malloc(sizeof(mmzk_list_t));
  struct node *dummy = malloc(sizeof(struct node));
  struct node *node1 = list->node;
  struct node *node = dummy;
  INIT_LIST(list->funs, list->is_persistent, result);

  while (node1 != NULL) {
    if (predicate(node1->elem)) {
      result->length++;
      node->next = malloc(sizeof(struct node));
      const void *copy = list->funs.copy_fun(node1->elem);
      node->next->elem = copy;
      node->next->prev_count = 0;
      node = node->next;
    }
    node1 = node1->next;
  }
  node->next = NULL;
  result->node = dummy->next;
  free(dummy);

  if (!list->is_persistent) {
    mmzk_list_free(list);
  }

  return result;
}

void *mmzk_list_fold_left(void *(*worker)(void *, const void *), void *init, mmzk_list_t *list) {
  void *result = init;
  size_t len = list->length;
  struct node *node = list->node;

  while (len > 0) {
    result = worker(result, node->elem);
    node = node->next;
    len--;
  }

  if (!list->is_persistent) {
    mmzk_list_free(list);
  }

  return result;
}

void *mmzk_list_fold_right(void *(*worker)(const void *, void *), void *init, mmzk_list_t *list) {
  void *result = _fold(list->length, worker, init, list->node);
  if (!list->is_persistent) {
    mmzk_list_free(list);
  }

  return result;
}


/* Iteration */

mmzk_list_iterator_t mmzk_list_iterator(mmzk_list_t *list) {
  return (mmzk_list_iterator_t) { .length = list->length, .node = list->node };
}

bool mmzk_list_has_next(mmzk_list_iterator_t iterator) {
  return iterator.length != 0;
}

void *mmzk_list_yield(mmzk_list_iterator_t *iterator) {
  void *elem = (void *)iterator->node->elem;
  *iterator = (mmzk_list_iterator_t) { .length = iterator->length - 1, .node = iterator->node->next };
  return elem;
}
