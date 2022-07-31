#ifndef MMZK1526
#define MMZK1526
#endif /* MMZK1526 */

#ifndef MMZK_LLIST_H
#define MMZK_LLIST_H

#include <stdint.h>
#include "mmzklist_base.h"


/* Construction & Destruction */

// New empty list, i.e. [].
// O(1).
mmzk_llist_t *mmzk_llist_new(mmzk_funs_t funs);

// Make list from array.
// Since the array is consumed lazily, it MUST NOT be deallocated before the list does.
// O(1).
mmzk_llist_t *mmzk_llist_from_array(mmzk_funs_t funs, size_t len, const void *elems[]);

// Construct list from the generating function GENERATOR, which produces the next element from the current one, starting
// from SEED.
// Note that SEED itself is not an element of the list, it's simply provided to GENERATOR to generate the first element.
// GENERATOR must not free its input and must allocate a new instance for the result.
// O(1).
mmzk_llist_t *mmzk_llist_from_generator(mmzk_funs_t funs, mmzk_pure_gen_t *generator, const void *seed);

// Free the list.
//
// Any list returned by the functions in this module must be freed even if the data may be shared.
// For example, suppose there is a list L1, and mmzk_llist_tail(L1) produces L2, then both L1 and L2 need to be freed
// despite L2 is fully contained within L1.
void mmzk_llist_free(mmzk_llist_t *list);

// Construct an identical list from LIST.
mmzk_llist_t *mmzk_llist_copy(mmzk_llist_t *list);

#endif /* MMZK_LLIST_H */
