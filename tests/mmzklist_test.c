#include <iso646.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../mmzklist.h"
#include "mmzktestbase.h"

#define MKINT(I) int32_t*_##I=malloc(sizeof(int32_t));do{*_##I=I;}while(0)
#define FRINT(I) int_free(_##I)
#define CHKELM(E, L, I) do{char *__mmzk_str=malloc(100);sprintf(__mmzk_str, "\telem check for %s @ %d: ", #L, I);void*__mmzk=mmzk_list_get(L,(size_t)I);mmzk_assert_equal_int32(E,*(int*)__mmzk,__mmzk_str);int_free(__mmzk);free(__mmzk_str);}while(0)

static void **make_range(int32_t i, int32_t j) {
  int32_t **result = malloc((j - i + 1) * sizeof(int32_t *));

  for (int32_t k = i; k <= j; k++) {
    result[k - i] = malloc(sizeof(int32_t *));
    *result[k - i] = k;
  }

  return (void **)result;
}

static void free_arr(void **range, int32_t len) {
  int32_t **result = (int32_t **)range;

  for (int32_t i = 0; i < len; i++) {
    free(result[i]);
  }

  free(result);
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

static bool less_than_five(const void *i1) {
  return *(int32_t *)i1 < 5;
}

static mmzk_funs_t int_funs = (mmzk_funs_t){&int_eq, &int_copy, &int_free};

static void construction_test(void) {
  {
    mmzk_assert_pop_caption("Can construct empty list and turn it into array:\n");
    mmzk_list_t *empty_list = mmzk_list_new(int_funs);
    size_t len = 114514;
    mmzk_assert_equal_int32(0, mmzk_list_length(empty_list), "\tlength empty_list == 0: ");

    void **empty_arr = mmzk_list_to_array(empty_list, NULL, &len);
    mmzk_assert_equal_int32(0, len, "\tlength empty_arr == 0: ");

    mmzk_list_free(empty_list);
    free(empty_arr);
    mmzk_assert_pop_caption("\n");
  }

  {
    mmzk_assert_pop_caption("Can construct list from empty array and turn it into array:\n");
    void **elems = malloc(0);
    mmzk_list_t *empty_list = mmzk_list_from_array(int_funs, 0, elems);
    size_t len = 114514;
    mmzk_assert_equal_int32(0, mmzk_list_length(empty_list), "\tlength empty_list == 0: ");

    void **empty_arr = mmzk_list_to_array(empty_list, NULL, &len);
    mmzk_assert_equal_int32(0, len, "\tlength empty_arr == 0: ");

    mmzk_list_free(empty_list);
    free(empty_arr);
    free(elems);
    mmzk_assert_pop_caption("\n");
  }

  {
    mmzk_assert_pop_caption("Can construct list from non-empty array and turn it into array:\n");
    void **_1_10 = make_range(1, 10);
    mmzk_list_t *one_to_ten = mmzk_list_from_array(int_funs, 10, _1_10);
    size_t len = 114514;
    mmzk_assert_equal_int32(10, mmzk_list_length(one_to_ten), "\tlength one_to_ten == 10: ");

    for (int32_t i = 0; i < 10; i++) {
      CHKELM(i + 1, one_to_ten, i);
    }

    void **arr = mmzk_list_to_array(one_to_ten, NULL, &len);
    mmzk_assert_equal_int32(10, len, "\tlength arr == 10: ");

    for (int32_t i = 0; i < 10; i++) {
      mmzk_assert_equal_int32(*((int32_t **)_1_10)[i], *((int32_t **)arr)[i], "\tarray element check: ");
    }

    mmzk_list_free(one_to_ten);
    free_arr(arr, 10);
    free_arr(_1_10, 10);
    mmzk_assert_pop_caption("\n");
  }

  {
    mmzk_assert_pop_caption("Can construct list from non-empty array and copy it:\n");
    void **_1_10 = make_range(1, 10);
    mmzk_list_t *one_to_ten = mmzk_list_from_array(int_funs, 10, _1_10);
    mmzk_assert_equal_int32(10, mmzk_list_length(one_to_ten), "\tlength one_to_ten == 10: ");

    for (int32_t i = 0; i < 10; i++) {
      CHKELM(i + 1, one_to_ten, i);
    }

    mmzk_list_t *one_to_ten_copy = mmzk_list_copy(one_to_ten);
    mmzk_assert_equal_int32(10, mmzk_list_length(one_to_ten_copy), "\tlength one_to_ten_copy == 10: ");
    for (int32_t i = 0; i < 10; i++) {
      CHKELM(i + 1, one_to_ten_copy, i);
    }

    mmzk_list_free(one_to_ten);
    mmzk_list_free(one_to_ten_copy);
    free_arr(_1_10, 10);
    mmzk_assert_pop_caption("\n");
  }
}

static void composition_test(void) {
  mmzk_list_t *nil = mmzk_list_new(int_funs);
  MKINT(1);
  MKINT(2);
  MKINT(3);
  mmzk_list_set_persistence(nil, false);
  mmzk_list_t *one_to_three = mmzk_list_cons(_1, mmzk_list_cons(_2, mmzk_list_cons(_3, nil)));
  mmzk_list_set_persistence(one_to_three, true);
  void **_4_6 = make_range(4, 6);
  mmzk_list_t *four_to_six = mmzk_list_from_array(int_funs, 3, _4_6);
  free_arr(_4_6, 3);

  {
    mmzk_assert_pop_caption("Can prepend elements to empty list:\n");
    for (int32_t i = 0; i < 3; i++) {
      CHKELM(i + 1, one_to_three, i);
    }
    mmzk_assert_pop_caption("\n");
  }

  {
    mmzk_assert_pop_caption("Can prepend elements to non-empty list:\n");
    mmzk_list_t *four_to_six_copy = mmzk_list_copy(four_to_six);
    mmzk_list_set_persistence(four_to_six_copy, false);
    mmzk_list_t *one_to_six = mmzk_list_cons(_1, mmzk_list_cons(_2, mmzk_list_cons(_3, four_to_six_copy)));
    mmzk_list_set_persistence(one_to_six, true);
    for (int32_t i = 0; i < 6; i++) {
      CHKELM(i + 1, one_to_six, i);
    }

    mmzk_list_free(one_to_six);
    mmzk_assert_pop_caption("\n");
  }

  {
    mmzk_assert_pop_caption("Can concatenate list:\n");
    mmzk_list_t *one_to_six = mmzk_list_concat(one_to_three, four_to_six);
    mmzk_list_t *rev_concat = mmzk_list_concat(four_to_six, one_to_three);
    for (int32_t i = 0; i < 6; i++) {
      CHKELM(i + 1, one_to_six, i);
    }
    
    for (int32_t i = 0; i < 3; i++) {
      CHKELM(i + 4, rev_concat, i);
    }
     for (int32_t i = 3; i < 6; i++) {
      CHKELM(i - 2, rev_concat, i);
    }

    mmzk_list_free(one_to_six);
    mmzk_list_free(rev_concat);
    mmzk_assert_pop_caption("\n");
  }

  FRINT(1);
  FRINT(2);
  FRINT(3);
  mmzk_list_free(one_to_three);
  mmzk_list_free(four_to_six);
}

static void take_drop_test(void) {
  mmzk_list_t *nil = mmzk_list_new(int_funs);
  void **_1_10 = make_range(1, 10);
  mmzk_list_t *one_to_ten = mmzk_list_from_array(int_funs, 10, _1_10);
  free_arr(_1_10, 10);

  {
    mmzk_assert_pop_caption("Can take head, tail, init, and last of non-empty list:\n");
    void *head = mmzk_list_head(one_to_ten);
    mmzk_list_t *tail = mmzk_list_tail(one_to_ten);
    mmzk_list_t *init = mmzk_list_init(one_to_ten);
    void *last = mmzk_list_last(one_to_ten);
    mmzk_assert_equal_int32(1, *(int32_t *)head, "\thead check: ");

    mmzk_assert_equal_int32(9, mmzk_list_length(tail), "\tlength tail == 9: ");
    for (int32_t i = 0; i < 9; i++) {
      CHKELM(i + 2, tail, i);
    }

    mmzk_assert_equal_int32(9, mmzk_list_length(init), "\tlength init == 9: ");
    for (int32_t i = 0; i < 9; i++) {
      CHKELM(i + 1, init, i);
    }

    mmzk_assert_equal_int32(10, *(int32_t *)last, "\tlast check: ");

    int_free(head);
    int_free(last);
    mmzk_list_free(tail);
    mmzk_list_free(init);
    mmzk_assert_pop_caption("\n");
  }

  {
    mmzk_assert_pop_caption("For empty list, head, tail, init, and last are NULL:\n");
    const void *head = mmzk_list_head(nil);
    mmzk_list_t *tail = mmzk_list_tail(nil);
    mmzk_list_t *init = mmzk_list_init(nil);
    const void *last = mmzk_list_last(nil);
    mmzk_assert_equal_ptr(NULL, head, "\thead is NULL: ");
    mmzk_assert_equal_ptr(NULL, tail, "\ttail is NULL: ");
    mmzk_assert_equal_ptr(NULL, init, "\tinit is NULL: ");
    mmzk_assert_equal_ptr(NULL, last, "\tlast is NULL: ");
    mmzk_assert_pop_caption("\n");
  }

  {
    mmzk_assert_pop_caption("take and drop of non-empty list:\n");
    mmzk_list_t *take0 = mmzk_list_take(0, one_to_ten);
    mmzk_list_t *drop0 = mmzk_list_drop(0, one_to_ten);
    mmzk_list_t *take4 = mmzk_list_take(4, one_to_ten);
    mmzk_list_t *drop4 = mmzk_list_drop(4, one_to_ten);
    mmzk_list_t *take10 = mmzk_list_take(10, one_to_ten);
    mmzk_list_t *drop10 = mmzk_list_drop(10, one_to_ten);
    mmzk_list_t *take14 = mmzk_list_take(14, one_to_ten);
    mmzk_list_t *drop14 = mmzk_list_drop(14, one_to_ten);

    mmzk_assert_equal_int32(0, mmzk_list_length(take0), "\tlength take0 == 0: ");
    mmzk_assert_equal_int32(4, mmzk_list_length(take4), "\tlength take4 == 4: ");
    mmzk_assert_equal_int32(10, mmzk_list_length(take10), "\tlength take10 == 10: ");
    mmzk_assert_equal_int32(10, mmzk_list_length(take14), "\tlength take14 == 10: ");

    mmzk_assert_equal_int32(10, mmzk_list_length(drop0), "\tlength drop0 == 10: ");
    mmzk_assert_equal_int32(6, mmzk_list_length(drop4), "\tlength drop4 == 6: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(drop10), "\tlength drop10 == 0: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(drop14), "\tlength drop14 == 0: ");

    for (int32_t i = 0; i < 4; i++) {
      CHKELM(i + 1, take4, i);
    }

    for (int32_t i = 0; i < 6; i++) {
      CHKELM(i + 5, drop4, i);
    }

    mmzk_list_free(take0);
    mmzk_list_free(drop0);
    mmzk_list_free(take4);
    mmzk_list_free(drop4);
    mmzk_list_free(take10);
    mmzk_list_free(drop10);
    mmzk_list_free(take14);
    mmzk_list_free(drop14);
    mmzk_assert_pop_caption("\n");
  }

  {
    mmzk_assert_pop_caption("take_end and drop_end of non-empty list:\n");
    mmzk_list_t *take_end0 = mmzk_list_take_end(0, one_to_ten);
    mmzk_list_t *drop_end0 = mmzk_list_drop_end(0, one_to_ten);
    mmzk_list_t *take_end4 = mmzk_list_take_end(4, one_to_ten);
    mmzk_list_t *drop_end4 = mmzk_list_drop_end(4, one_to_ten);
    mmzk_list_t *take_end10 = mmzk_list_take_end(10, one_to_ten);
    mmzk_list_t *drop_end10 = mmzk_list_drop_end(10, one_to_ten);
    mmzk_list_t *take_end14 = mmzk_list_take_end(14, one_to_ten);
    mmzk_list_t *drop_end14 = mmzk_list_drop_end(14, one_to_ten);

    mmzk_assert_equal_int32(0, mmzk_list_length(take_end0), "\tlength take_end0 == 0: ");
    mmzk_assert_equal_int32(4, mmzk_list_length(take_end4), "\tlength take_end4 == 4: ");
    mmzk_assert_equal_int32(10, mmzk_list_length(take_end10), "\tlength take_end10 == 10: ");
    mmzk_assert_equal_int32(10, mmzk_list_length(take_end14), "\tlength take_end14 == 10: ");

    mmzk_assert_equal_int32(10, mmzk_list_length(drop_end0), "\tlength drop_end0 == 10: ");
    mmzk_assert_equal_int32(6, mmzk_list_length(drop_end4), "\tlength drop_end4 == 6: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(drop_end10), "\tlength drop_end10 == 0: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(drop_end14), "\tlength drop_end14 == 0: ");

    for (int32_t i = 0; i < 4; i++) {
      CHKELM(i + 7, take_end4, i);
    }

    for (int32_t i = 0; i < 6; i++) {
      CHKELM(i + 1, drop_end4, i);
    }

    mmzk_list_free(take_end0);
    mmzk_list_free(drop_end0);
    mmzk_list_free(take_end4);
    mmzk_list_free(drop_end4);
    mmzk_list_free(take_end10);
    mmzk_list_free(drop_end10);
    mmzk_list_free(take_end14);
    mmzk_list_free(drop_end14);
    mmzk_assert_pop_caption("\n");
  }

  {
    mmzk_assert_pop_caption("For empty lists, take, drop, take_end, and drop_end are empty:\n");
    mmzk_list_t *take0 = mmzk_list_take(0, nil);
    mmzk_list_t *drop0 = mmzk_list_drop(0, nil);
    mmzk_list_t *take_end0 = mmzk_list_take_end(0, nil);
    mmzk_list_t *drop_end0 = mmzk_list_drop_end(0, nil);
    mmzk_assert_equal_int32(0, mmzk_list_length(take0), "\tlength take0 == 0: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(drop0), "\tlength drop0 == 0: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(take_end0), "\tlength take_end0 == 0: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(drop_end0), "\tlength drop_end0 == 0: ");
    mmzk_list_free(take0);
    mmzk_list_free(drop0);
    mmzk_list_free(take_end0);
    mmzk_list_free(drop_end0);
    mmzk_assert_pop_caption("\n");
  }

  mmzk_list_free(nil);
  mmzk_list_free(one_to_ten);
}

static void split_span_test(void) {
  mmzk_list_t *nil = mmzk_list_new(int_funs);
  void **_1_10 = make_range(1, 10);
  mmzk_list_t *one_to_ten = mmzk_list_from_array(int_funs, 10, _1_10);
  free_arr(_1_10, 10);

  {
    mmzk_assert_pop_caption("Can use split & split_end on non-empty list:\n");
    mmzk_list_tuple_t split0 = mmzk_list_split_at(0, one_to_ten);
    mmzk_list_tuple_t split_end0 = mmzk_list_split_at_end(0, one_to_ten);
    mmzk_list_tuple_t split4 = mmzk_list_split_at(4, one_to_ten);
    mmzk_list_tuple_t split_end4 = mmzk_list_split_at_end(4, one_to_ten);
    mmzk_list_tuple_t split10 = mmzk_list_split_at(10, one_to_ten);
    mmzk_list_tuple_t split_end10 = mmzk_list_split_at_end(10, one_to_ten);
    mmzk_list_tuple_t split14 = mmzk_list_split_at(14, one_to_ten);
    mmzk_list_tuple_t split_end14 = mmzk_list_split_at_end(14, one_to_ten);

    mmzk_assert_equal_int32(0, mmzk_list_length(split0.fst), "\flength $ fst split0 == 0: ");
    mmzk_assert_equal_int32(10, mmzk_list_length(split0.snd), "\flength $ snd split0 == 10: ");
    mmzk_assert_equal_int32(4, mmzk_list_length(split4.fst), "\flength $ fst split4 == 4: ");
    mmzk_assert_equal_int32(6, mmzk_list_length(split4.snd), "\flength $ snd split4 == 6: ");
    mmzk_assert_equal_int32(10, mmzk_list_length(split10.fst), "\flength $ fst split10 == 10: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(split10.snd), "\flength $ snd split10 == 0: ");
    mmzk_assert_equal_int32(10, mmzk_list_length(split14.fst), "\flength $ fst split14 == 10: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(split14.snd), "\flength $ snd split14 == 0: ");

    mmzk_assert_equal_int32(10, mmzk_list_length(split_end0.fst), "\flength $ fst split_end0 == 10: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(split_end0.snd), "\flength $ snd split_end0 == 0: ");
    mmzk_assert_equal_int32(6, mmzk_list_length(split_end4.fst), "\flength $ fst split_end4 == 6: ");
    mmzk_assert_equal_int32(4, mmzk_list_length(split_end4.snd), "\flength $ snd split_end4 == 4: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(split_end10.fst), "\flength $ fst split_end10 == 0: ");
    mmzk_assert_equal_int32(10, mmzk_list_length(split_end10.snd), "\flength $ snd split_end10 == 10: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(split_end14.fst), "\flength $ fst split_end14 == 0: ");
    mmzk_assert_equal_int32(10, mmzk_list_length(split_end14.snd), "\flength $ snd split_end14 == 10: ");

    for (int32_t i = 0; i < 4; i++) {
      CHKELM(i + 1, split4.fst, i);
    }
    for (int32_t i = 0; i < 6; i++) {
      CHKELM(i + 5, split4.snd, i);
    }

    for (int32_t i = 0; i < 6; i++) {
      CHKELM(i + 1, split_end4.fst, i);
    }
    for (int32_t i = 0; i < 4; i++) {
      CHKELM(i + 7, split_end4.snd, i);
    }

    mmzk_list_free(split0.fst);
    mmzk_list_free(split0.snd);
    mmzk_list_free(split_end0.fst);
    mmzk_list_free(split_end0.snd);
    mmzk_list_free(split4.fst);
    mmzk_list_free(split4.snd);
    mmzk_list_free(split_end4.fst);
    mmzk_list_free(split_end4.snd);
    mmzk_list_free(split10.fst);
    mmzk_list_free(split10.snd);
    mmzk_list_free(split_end10.fst);
    mmzk_list_free(split_end10.snd);
    mmzk_list_free(split14.fst);
    mmzk_list_free(split14.snd);
    mmzk_list_free(split_end14.fst);
    mmzk_list_free(split_end14.snd);
  }

    {
    mmzk_assert_pop_caption("Can use span on non-empty list:\n");
    mmzk_list_tuple_t spanned = mmzk_list_span(&less_than_five, one_to_ten);

    mmzk_assert_equal_int32(4, mmzk_list_length(spanned.fst), "\flength $ fst spanned == 4: ");
    mmzk_assert_equal_int32(6, mmzk_list_length(spanned.snd), "\flength $ snd split0 == 6: ");

    for (int32_t i = 0; i < 4; i++) {
      CHKELM(i + 1, spanned.fst, i);
    }
    for (int32_t i = 0; i < 6; i++) {
      CHKELM(i + 5, spanned.snd, i);
    }

    mmzk_list_free(spanned.fst);
    mmzk_list_free(spanned.snd);
  }

  {
    mmzk_assert_pop_caption("For empty lists, split, split_end, and span are empty:\n");
    mmzk_list_tuple_t split0 = mmzk_list_split_at(0, nil);
    mmzk_list_tuple_t split_end0 = mmzk_list_split_at_end(0, nil);
    mmzk_list_tuple_t spanned = mmzk_list_span(&less_than_five, nil);
    mmzk_assert_equal_int32(0, mmzk_list_length(split0.fst), "\tlength $ fst split0 == 0: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(split0.snd), "\tlength $ snd split0 == 0: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(split_end0.fst), "\tlength $ fst split_end0 == 0: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(split_end0.snd), "\tlength $ snd split_end0 == 0: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(spanned.fst), "\tlength $ fst spanned == 0: ");
    mmzk_assert_equal_int32(0, mmzk_list_length(spanned.snd), "\tlength $ snd spanned == 0: ");
    mmzk_list_free(split0.fst);
    mmzk_list_free(split0.snd);
    mmzk_list_free(split_end0.fst);
    mmzk_list_free(split_end0.snd);
    mmzk_list_free(spanned.fst);
    mmzk_list_free(spanned.snd);
    mmzk_assert_pop_caption("\n");
  }

  mmzk_list_free(nil);
  mmzk_list_free(one_to_ten);
}

static void test_summary(void) {
  mmzk_test_summary(construction_test, "Test list construction and array conversion:\n");
  mmzk_test_summary(composition_test, "Test list prepending and concatenation:\n");
  mmzk_test_summary(take_drop_test, "Test take/drop functions:\n");
  mmzk_test_summary(split_span_test, "Test split/span functions:\n");
}

int32_t main(int32_t argc, char **argv) {
  return mmzk_test_report(test_summary, argc, argv);
}
