# Functional Data Structures in C
TODO

Note that these constructs made in C are only for fun; they are usually less efficient than their Haskell counterparts, mainly due to an extensive use of reference counting style of memory management.

## Lists

### Introduction
The functional [List](https://www.codurance.com/publications/2018/04/27/lists) is one of the most iconic and important data structures in functional programming. In Haskell, it is defined as a recursive data type:

```hs
-- Of course, in Haskell we actually use "[]" for "Nil" and "(:)" for "Cons", but those are just syntax sugars.
data List a = Nil | Cons a (List a)
```

Theoretically, all operations with lists can be expressed via `Nil` (namely `[]`), `Cons` (namely `(:)`), pattern matching, and recursion. For example, to get all elements in a list except the last one:

```hs
init (Cons a Nil) = a
init (Cons a as)  = Cons a (last as)
```

However, there are two more niches in the actual implementation of such structure, the first one is universal among all persistent data structures, while the second is more of a C-specific problem. We will discuss both.

Fistly, we can achieve better time complexity for operations by sharing some of the data carefully. If we implement Lists naively, then every `Cons` requires copying the original List. For example, consider a List `l1 = [3, 4, 5]` (which is `Cons 3 (Cons 4 (Cons 5 Nil))`, but we will written it in the more "natural" way of expressing Lists), and we prepend it with the element `2`. By definition, we would create a new copy of `[3, 4, 5]` and put `2` in front of it, producing a `l2 = [2, 3, 4, 5]` while preserving the original `[3, 4, 5]`:

```diagram
l1:      3 -> 4 -> 5 -> Nil
l2: 2 -> 3 -> 4 -> 5 -> Nil
```

This implementation has a time complexity of `O(n)`. With sharing, however, we can simply reuse `l1`, and since the elements in Lists are immutable, we don't have to worry about affecting `l2` by changing the elements in `l1`:

```diagram
l2   l1
|    |
V    V
2 -> 3 -> 4 -> 5 -> Nil
```

With this approach, we achieved O(1) time complexity for `Cons`.

Take concatenation as another example. Suppose we have `l1 = [1, 2, 3]` and `l2 = [4, 5, 6]`. If we want to concatenate them together, *i.e* `l3 = l1 ++ l2`, a solution with naïve copy would copy both `l1` and `l2`, making the time complexity O(n + m). However, with [path copying](https://en.wikipedia.org/wiki/Persistent_data_structure#Path_copying), we only need to copy `l1`; `l2` can be shared with the concatenated list:

```diagram
l1
|
V
1 -> 2 -> 3 -> nil

l3             l2
|              |
V              V
1 -> 2 -> 3 -> 4 -> 5 -> 6 -> nil
```

So that the time complexity is O(n) (the length of the left List). There are other techniques we could apply, such as fat pointers, but they bring small overheads on accessing Lists as the number of operations on the List grows. A combination of both fat nodes and path copying can produce amortised `O(1)` space and `O(1)` extra time.

The second problem is directly related to the inability to collect garbages of the C programming language. Take the `Cons` example again: `l1 = [3, 4, 5]` and `l2 = 2 : [3, 4, 5]`. We knew that only one List actually exists in memory, thus deallocating `l1` would also deallocate the tail of `l2`, which is undesirable since `l1` and `l2` are semantically unrelated.

A closely related issue is about managing the actual elements. Naturally, I use `void *` to represent the element type to achieve generics, and there are many tricky questions regarding how elements are copied and deallocated. Should I deep-copy the element before putting it into a List? Should I deallocate an element when its node is deallocated? How can I ensure that the original elements are still accessible after the List containing it is deallocated? Ideally, a uniform protocol is prefered:

* Each List is provided with user-defined `copy_fun`, `free_fun` and `eq_fun`.
* `copy_fun` takes one `void *` argument and returns `void *`.
* `eq_fun` takes two `void *` arguments and returns `bool`. An element must be equal to its copy, *i.e.* `eq_fun(a, a_copy)` is `true` if `void *a_copy = copy_fun(a)`;
* `free_fun` takes one `void *` argument. If all elements and there copies are applied to `free_fun`, there should be neither memory leak nor double free.

Until now, we are still not dealing with actual Lists. Instead, I simply declared three functions and required them to satisfy certain rules. Note that if `copy_fun` is a deep copy, `free_fun` is a recursive free, and `eq_fun` is structural equality, then it would satisfy the laws. However, in many circustances, deep-copying is expensive, and we can simply attach a counter to our element so that `copy_fun` increments it and `free_fun` decrements it, only deallocates when the counter is zero. In this case, the element is not really "copied" and its modification can cause problems in Lists that contain it. Therefore, we have:

* Elements or copies of them that are provided to Lists should not be modified during the lifespan of these Lists.

Finally, there are two more rules for the actual Lists:

* Unless otherwise specified, all Lists must be passed to the List-freeing function (`mmzk_list_free()` for strict Lists and `mmzk_llist_free()` for lazy ones) at the end of their lifespans.
* Any element put into or retrieved from a List (for example, via `mmzk_list_get()`) must be explicitly deallocated.

For example, with `l1 = [3, 4, 5]` and `l2 = 2 : l2` (here we used the Haskell notation for simplicity; real C Lists involve functions such as `mmzk_list_from_array()` and `mmzk_list_cons()`), we need to apply the List-freeing function on both Lists despite `l1` is fully contained within `l2`; if we have `elem = mmzk_list_head(l2)`, then we do not free `elem` explicitly.

Whenever the protocols above are satisfied, memory safety is ensured. I implemented this using some internal metadata as well as user-provided functions, see the following sections for details.

### Example
TODO: Explain the power set example.
