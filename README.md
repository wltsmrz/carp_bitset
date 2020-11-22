# carp_bitset

Bitsets for carp - mostly in C and borrowed from Mr. Daniel Lemire's [cbitset](https://github.com/lemire/cbitset/), cleaned up and made Carp-like

```clojure
(load "https://github.com/wltsmrz/carp_bitset@v0.0.1")
(load "Test.carp")
(use Test)

(def s1 (BitSet.from-array &[ (Uint64.from-long 1l) (Uint64.from-long 3l) (Uint64.from-long 5l) ]))
(def s2 (BitSet.from-array &[ (Uint64.from-long 2l) (Uint64.from-long 4l) (Uint64.from-long 6l) ]))
(def s3 (BitSet.from-array &[ (Uint64.from-long 2l) (Uint64.from-long 32l) ]))
(def s4 (BitSet.from-array &[ ]))
(def s5 (BitSet.from-array &[ (Uint64.from-long 1l) (Uint64.from-long 3l) (Uint64.from-long 5l) ]))

(defn main []
  (with-test test
    (assert-equal test 3 (BitSet.card &s1) "BitSet.card")
    (assert-equal test 3 (BitSet.card &s2) "BitSet.card")
    (assert-equal test 2 (BitSet.card &s3) "BitSet.card")
    (assert-equal test 0 (BitSet.card &s4) "BitSet.card")

    (assert-equal test 1 (BitSet.min &s1) "BitSet.min")
    (assert-equal test 2 (BitSet.min &s2) "BitSet.min")
    (assert-equal test 2 (BitSet.min &s3) "BitSet.min")

    (assert-equal test 5 (BitSet.max &s1) "BitSet.max")
    (assert-equal test 6 (BitSet.max &s2) "BitSet.max")
    (assert-equal test 32 (BitSet.max &s3) "BitSet.max")

    (assert-equal test true (BitSet.contains? &s1 (Uint64.from-long 1l)) "BitSet.contains?")
    (assert-equal test false (BitSet.contains? &s1 (Uint64.from-long 2l)) "BitSet.contains?")
    (assert-equal test false (BitSet.contains? &s4 (Uint64.from-long 2l)) "BitSet.contains?")

    (assert-equal test true (BitSet.disjoint? &s1 &s3) "BitSet.disjoint?")
    (assert-equal test false (BitSet.disjoint? &s2 &s3) "BitSet.disjoint?")
    (assert-equal test true (BitSet.disjoint? &s1 &s4) "BitSet.disjoint?")

    (assert-equal test true (BitSet.intersect? &s2 &s3) "BitSet.intersect?")
    (assert-equal test false (BitSet.intersect? &s1 &s3) "BitSet.intersect?")
    (assert-equal test false (BitSet.intersect? &s4 &s3) "BitSet.intersect?")

    (assert-equal test true (BitSet.contains-all? &s1 &s5) "BitSet.contains-all?")
    (assert-equal test false (BitSet.contains-all? &s1 &s3) "BitSet.contains-all?")
    (assert-equal test false (BitSet.contains-all? &s4 &s5) "BitSet.contains-all?")

    (assert-equal test 6 (BitSet.union-card &s1 &s2) "BitSet.union-card")
    (assert-equal test 5 (BitSet.union-card &s1 &s3) "BitSet.union-card")
    (assert-equal test 3 (BitSet.union-card &s1 &s4) "BitSet.union-card")

    (assert-equal test 0 (BitSet.intersection-card &s1 &s2) "BitSet.intersection-card")
    (assert-equal test 0 (BitSet.intersection-card &s1 &s3) "BitSet.intersection-card")
    (assert-equal test 1 (BitSet.intersection-card &s2 &s3) "BitSet.intersection-card")

    (assert-equal test 3 (BitSet.difference-card &s1 &s2) "BitSet.difference-card")
    (assert-equal test 3 (BitSet.difference-card &s1 &s3) "BitSet.difference-card")
    (assert-equal test 2 (BitSet.difference-card &s2 &s3) "BitSet.difference-card")

    (assert-equal test 6 (BitSet.symmetric-difference-card &s1 &s2) "BitSet.symmetric-difference-card")
    (assert-equal test 5 (BitSet.symmetric-difference-card &s1 &s3) "BitSet.symmetric-difference-card")
    (assert-equal test 3 (BitSet.symmetric-difference-card &s2 &s3) "BitSet.symmetric-difference-card")

    (assert-equal test "BitSet{1 2 3 4 5 6}" &(BitSet.str &(BitSet.union &s1 &s2)) "BitSet.str(BitSet.union)")
    (assert-equal test "BitSet{1 2 3 5 32}" &(BitSet.str &(BitSet.union &s1 &s3)) "BitSet.str(BitSet.union)")
    (assert-equal test "BitSet{1 3 5}" &(BitSet.str &(BitSet.union &s1 &s4)) "BitSet.str(BitSet.union)")

    (assert-equal test "BitSet{}" &(BitSet.str &(BitSet.intersection &s1 &s2)) "BitSet.str(BitSet.intersection)")
    (assert-equal test "BitSet{2}" &(BitSet.str &(BitSet.intersection &s2 &s3)) "BitSet.str(BitSet.intersection)")
    (assert-equal test "BitSet{}" &(BitSet.str &(BitSet.intersection &s1 &s4)) "BitSet.str(BitSet.intersection)")

    (assert-equal test "BitSet{1 3 5}" &(BitSet.str &(BitSet.difference &s1 &s2)) "BitSet.str(BitSet.difference)")
    (assert-equal test "BitSet{4 6}" &(BitSet.str &(BitSet.difference &s2 &s3)) "BitSet.str(BitSet.difference)")
    (assert-equal test "BitSet{1 3 5}" &(BitSet.str &(BitSet.difference &s1 &s4)) "BitSet.str(BitSet.difference)")

    (assert-equal test "BitSet{1 2 3 4 5 6}" &(BitSet.str &(BitSet.symmetric-difference &s1 &s2)) "BitSet.str(BitSet.symmetric-difference)")
    (assert-equal test "BitSet{4 6 32}" &(BitSet.str &(BitSet.symmetric-difference &s2 &s3)) "BitSet.str(BitSet.symmetric-difference)")
    (assert-equal test "BitSet{1 3 5}" &(BitSet.str &(BitSet.symmetric-difference &s1 &s4)) "BitSet.str(BitSet.symmetric-difference)")
  )
)

```

