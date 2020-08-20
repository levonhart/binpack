binpack
=======

Binpack implements the first-fit algorithm and some heuristics for the
classical Bin Packing Problem, as well as comparisons between different
methods.

Definition
----------

The Bin Packing Problem consists in, given an integer **c**, a set of
items **S**, and for each item **i** a weight **w(i)**,
finding a partition of **S** with minimum cardinality such that
its elements are pair-wise disjoint and their load do not exceed the
capacity **c**.

Building
--------

### Prerequisites:
	- `gcc`,`clang` or another C compiler
	- `cmake >=3.15`
	- `ninja` build tool recommended

### Building instructions:
`git clone` this repository;
```sh
cd binpack
mkdir build
cd build
cmake ../			# -D"Ninja" -DCMAKE_C_COMPILER=clang
cmake --build .
```

### Testing
Run it to make sure it was compiled properly
```sh
ctest .
```
