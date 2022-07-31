#ifndef MMZK1526
#define MMZK1526
#endif /* MMZK1526 */

#ifndef MMZK_LIST_H
#define MMZK_LIST_H

#include <stdint.h>
#include "mmzklist_base.h"


/* Construction & Destruction */

// New empty list, i.e. [].
// O(1).
mmzk_list_t *mmzk_list_new(mmzk_funs_t funs);

// Make list from array.
// O(n).
mmzk_list_t *mmzk_list_from_array(mmzk_funs_t funs, size_t len, void *elems[]);

// Turn LIST into an array. The functions of LIST will be stored in FUNS (if not NULL) and the length will be stored in
// LEN (if not NULL).
// O(n).
void **mmzk_list_to_array(mmzk_list_t *list, mmzk_funs_t *funs, size_t *len);

// Free the list.
//
// Any list returned by the functions in this module must be freed even if the data may be shared.
// For example, suppose there is a list L1, and mmzk_list_tail(L1) produces L2, then both L1 and L2 need to be freed
// despite L2 is fully contained within L1.
void mmzk_list_free(mmzk_list_t *list);

// Construct an identical list from LIST.
// This function never deallocates LIST, regardless of its persistence state.
mmzk_list_t *mmzk_list_copy(mmzk_list_t *list);

// If PERSISTENCE is TRUE (by default), then passing LIST to another function in this module does not modify itself.
// Otherwise, LIST will be deallocated when used as an argument to a function (unless specified otherwise).
void mmzk_list_set_persistence(mmzk_list_t *list, bool persistence);


/* Query */

// The length of LIST, i.e. length LIST.
// O(1).
size_t mmzk_list_length(mmzk_list_t *list);

// If the LIST is empty, i.e. null LIST.
// O(1).
inline bool mmzk_list_is_empty(mmzk_list_t *list) {
    return mmzk_list_length(list) == 0;
}

// Get the INDEX-th element of LIST, NULL if out of bound, i.e. LIST !! INDEX.
// O(n).
void *mmzk_list_get(mmzk_list_t *list, size_t index);

// Get the INDEX-th element of LIST counted from the last element, NULL if out of bound,
// i.e. LIST !! (length LIST - INDEX - 1).
// O(n); only one iteration through the list.
void *mmzk_list_get_end(mmzk_list_t *list, size_t index);

// Get the first element of LIST, NULL if empty, i.e. head LIST.
// O(1).
static inline void *mmzk_list_head(mmzk_list_t *list) {
    return mmzk_list_get(list, 0);
}

// Get the last element of LIST, NULL if empty, i.e. last LIST.
// O(n).
static inline void *mmzk_list_last(mmzk_list_t *list) {
    return mmzk_list_get_end(list, 0);
}

// Whether ELEMENT is an element of LIST.
// This function never deallocates LIST, regardless of its persistence state.
// O(n).
bool mmzk_list_is_elem(const void *element, mmzk_list_t *list);

// Whether LIST1 and LIST2 are structurally equal.
// This function never deallocates LIST, regardless of its persistence state.
// O(n).
bool mmzk_list_equal(mmzk_list_t *list1, mmzk_list_t *list2);


/* Composition */

// Construct a list by prepending ELEM to the given LIST, i.e. ELEM : LIST.
// O(1).
mmzk_list_t *mmzk_list_cons(const void *elem, mmzk_list_t *list);

// Construct a list by concatenating LIST1 with LIST2, i.e. LIST1 ++ LIST2.
// LIST2 is shared in the new list while LIST1 is copied.
// O(n).
mmzk_list_t *mmzk_list_concat(mmzk_list_t *list1, mmzk_list_t *list2);


/* Decomposition */

// Get the list without the first element in LIST, NULL if empty, i.e. tail LIST.
// O(1).
void *mmzk_list_tail(mmzk_list_t *list);

// Get the list without the last element in LIST, NULL if empty, i.e. init LIST.
// O(1).
void *mmzk_list_init(mmzk_list_t *list);

// Take the first I elements in LIST, i.e. take I LIST.
// O(1).
void *mmzk_list_take(size_t i, mmzk_list_t *list);

// Drop the first I elements in LIST, i.e. drop I LIST.
// O(n).
void *mmzk_list_drop(size_t i, mmzk_list_t *list);

// Take the last I elements in LIST, i.e. drop (length LIST - I) LIST.
// O(n).
static inline void *mmzk_list_take_end(size_t i, mmzk_list_t *list) {
    size_t len = mmzk_list_length(list);
    return mmzk_list_drop(len > i ? len - i : 0, list);
}

// Drop the last I elements in LIST, i.e. take (length LIST - I) LIST.
// O(1).
static inline void *mmzk_list_drop_end(size_t i, mmzk_list_t *list) {
    size_t len = mmzk_list_length(list);
    return mmzk_list_take(len > i ? len - i : 0, list);
}

// Split LIST at index I, i.e. splitAt I LIST.
// O(n).
mmzk_list_tuple_t mmzk_list_split_at(size_t i, mmzk_list_t *list);

// Split LIST at index I counting from the right, i.e. splitAt (length LIST - I) LIST.
// O(n).
static inline mmzk_list_tuple_t mmzk_list_split_at_end(size_t i, mmzk_list_t *list) {
    size_t len = mmzk_list_length(list);
    return mmzk_list_split_at(len > i ? len - i : 0, list);
}

// Split LIST into the longest prefix where PREDICATE holds and the rest of the list, i.e. span PREDICATE LIST.
// O(n).
mmzk_list_tuple_t mmzk_list_span(predicate_t *predicate, mmzk_list_t *list);


/* Transformation */

// Transform LIST by applying WORKER on each element, i.e. map WORKER LIST.
// Inputs to WORKER are not copied, thus it is WORKER's responsibility to return a new instance.
// O(n) not considering the time complexity of WORKER.
mmzk_list_t *mmzk_list_map(mmzk_funs_t funs, void *(*worker)(const void *, void *), mmzk_list_t *list, void *);

// Returns a list containing all elements in LIST that satisfies PREDICATE, i.e. filter PREDICATE LIST.
// O(n) not considering the time complexity of PREDICATE.
mmzk_list_t *mmzk_list_filter(predicate_t *predicate, mmzk_list_t *list);

// Reduce the elements in LIST by WORKER from left to right, i.e. foldl WORKER INIT LIST.
// WORKER must take care of the lifespan of the accumulator argument (first), but should not modify or deallocate the
// list element argument (second).
// INIT should not be accessed after being passed to this function.
// O(n) not considering the time complexity of WORKER.
void *mmzk_list_fold_left(void *(*worker)(void *, const void *), void *init, mmzk_list_t *list);

// Reduce the elements in LIST by WORKER from right to left, i.e. foldr WORKER INIT LIST.
// WORKER must take care of the lifespan of the accumulator argument (second), but should not modify or deallocate the
// list element argument (first).
// INIT should not be accessed after being passed to this function.
// O(n) not considering the time complexity of WORKER.
void *mmzk_list_fold_right(void *(*worker)(const void *, void *), void *init, mmzk_list_t *list);


/* Iteration */

// Iteration outline:
// mmzk_list_t *list = ...;
// mmzk_list_iterator_t iter = mmzk_list_iterator(list);
// while (mmzk_list_has_next(iter)) {
//   void *elem = mmzk_list_yield(&iter);
//   ... do something with elem ...
// }

// Get the iterator for LIST. If the iterator NULL, then the end of the list is reached.
// It does not need to be deallocated, but must not be used after freeing LIST.
// O(1).
mmzk_list_iterator_t mmzk_list_iterator(mmzk_list_t *list);

// If the iterator still yields elements.
bool mmzk_list_has_next(mmzk_list_iterator_t iterator);

// Get the current element of the iterator and move to the next. Undefined behaviour if it does not have elements.
// O(1).
void *mmzk_list_yield(mmzk_list_iterator_t *iterator);

#endif /* MMZK_LIST_H */
