<p align=center><a href="https://seahorn.github.io"><img src="https://seahorn.github.io/images/seahorn-logo.png" alt="seahorn" width="200px" height="200px"/></a></p>

<table>
  <tr>
    <th>Windows</th><th>Ubuntu</th><th>OS X</th><th>Chat with us</th><th>Stories</th>
  </tr>
    <td>TBD</td>
    <td><a href="https://travis-ci.org/seahorn/seahorn"><img src="https://travis-ci.org/seahorn/seahorn.svg?branch=master" title="Ubuntu 12.04 LTS 64bit, g++-5"/></a></td>
    <td>TBD</td>
    <td><a href="https://gitter.im/seahorn/seahorn?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge"><img src="https://badges.gitter.im/seahorn/seahorn.svg" title="Gitter"/></a></td>
    <td> <a href="http://waffle.io/seahorn/seahorn/"><img src="https://badge.waffle.io/seahorn/seahorn.svg?label=ready&title=Ready"/></a></td>
  </tr>
</table>

# About #

<a href="http://seahorn.github.io/">SeaHorn</a> is an automated analysis framework for LLVM-based languages. LLVM version is 3.8.

# License #

<a href="http://seahorn.github.io/">SeaHorn</a> is distributed under a modified BSD license. See [license.txt](license.txt) for details.

# Installation #

* `cd seahorn ; mkdir build ; cd build`
* `cmake -DCMAKE_INSTALL_PREFIX=run ../ `
* `cmake --build .` to build dependencies (Z3 and LLVM)
* `cmake --build . --target extra && cmake ..` to download extra packages
* `cmake --build . --target crab && cmake ..` to configure crab-llvm (if `extra` target was run)
* `cmake --build . --target install` to build seahorn and install everything in `run` directory

_Note that the *install* target is required!_

The install target installs SeaHorn all of it dependencies under `build/run`.
The main executable is `build/run/bin/sea`.

SeaHorn provides several components that are installed via the `extra`
target. These components can be used by other projects outside of
SeaHorn.

* [llvm-seahorn](https://github.com/seahorn/llvm-seahorn): `git clone https://github.com/seahorn/llvm-seahorn.git`

  `llvm-dsa` is the legacy DSA implementation
  from [PoolAlloc](https://llvm.org/svn/llvm-project/poolalloc/). DSA
  is a heap analysis used by SeaHorn to disambiguate the heap.

* [sea-dsa](https://github.com/seahorn/sea-dsa): `git clone https://github.com/seahorn/sea-dsa.git`

  `sea-dsa` is a new DSA-based heap analysis. Unlike `llvm-dsa`,
  `sea-dsa` is context-sensitive and therefore, a finer-grained
  partition of the heap can be generated in presence of function
  calls.

* [crab-llvm](https://github.com/seahorn/crab-llvm): `git clone https://github.com/seahorn/crab-llvm.git`

  `crab-llvm` provides inductive invariants using abstract
  interpretation techniques to the rest of SeaHorn's backends.

* [llvm-seahorn](https://github.com/seahorn/llvm-seahorn): `git clone https://github.com/seahorn/llvm-seahorn.git`

  `llvm-seahorn` provides tailored-to-verification versions of
  `InstCombine` and `IndVarSimplify` LLVM passes as well as a LLVM
  pass to convert undefined values into nondeterministic calls, among
  other things.

SeaHorn doesn't come with its own version of Clang and expects to find it 
either in the build directory (`run/bin`) or in PATH. Make sure that the 
version of Clang matches the version of LLVM that comes with SeaHorn 
(currently 3.8). The easiest way to provide the right version of Clang is 
to download it from [llvm.org](http://releases.llvm.org/download.html), 
unpact it somewhere and create a symbolic link to `clang` and `clang++` 
in `run/bin`.
```
cd seahorn/build/run/bin
ln -s place_where_you_unpacked_clang/bin/clang clang
ln -s place_where_you_unpacked_clang/bin/clang++ clang++
```

# Test #

Tests require `lit` and `OutputCheck`. These can be installed using

``` shell
pip install lit OutputCheck
```

Test can be run using

``` shell
  $ cmake --build . --target test-simple
  $ cmake --build . --target test-solve
  $ cmake --build . --target test-abc
  $ cmake --build . --target test-dsa
```

__Note: `test-dsa` requires additional python packages__


# Usage #

SeaHorn provides a python script called `sea` to interact with
users. Given a C program annotated with assertions, users just need to
type: `sea pf file.c`

This will output `unsat` if all assertions hold or otherwise `sat` if
any of the assertions is violated.

The option `pf` tells SeaHorn to translate `file.c` into LLVM
bitecode, generate a set of verification conditions (VCs), and
finally, solve them. The main back-end solver
is [spacer](https://bitbucket.org/spacer/).


The command `pf` provides, among others, the following options:

- `--show-invars`: display computed invariants if answer was `unsat`.

- `--cex=FILE` : stores a counter-example in `FILE` if answer was `sat`.

- `-g` : compiles with debug information for more trackable
  counterexamples.

- `--step=large`: large-step encoding. Each transition relation
corresponds to a loop-free fragments.

- `--step=small`: small-step encoding. Each transition relation
  corresponds to a basic block.

- `--track=reg` : model (integer) registers only.

- `--track=ptr` : model registers and pointers (but not memory content)

- `--track=mem`: model both scalars, pointers, and memory contents

- `--inline` : inlines the program before verification

- `--crab` : inject invariants in `spacer` generated by the Crab
  abstract-interpretation-based
  tool. Read
  [here](https://github.com/seahorn/crab-llvm/tree/master#crab-options) for
  details about all Crab options (prefix `--crab`). You can see which
  invariants are inferred by Crab by typing option `--log=crab`.

- `--bmc`: use BMC engine.


`sea pf` is a pipeline that runs multiple commands. Individual parts
of the pipeline can be run separately as well:

1. `sea fe file.c -o file.bc`: SeaHorn frontend translates a C program
  into optimized LLVM bitcode including mixed-semantics
  transformation.

2. `sea horn file.bc -o file.smt2`: SeaHorn generates the verification
  conditions from `file.bc` and outputs them into SMT-LIB v2 format. Users
  can choose between different encoding styles with several levels of
  precision by adding:

   - `--step={small,large,fsmall,flarge}` where `small` is small step
      encoding, `large` is block-large encoding, `fsmall` is small
      step encoding producing flat Horn clauses (i.e., it generates a
      transition system with only one predicate), and `flarge`:
      block-large encoding producing flat Horn clauses.

   - `--track={reg,ptr,mem}` where `reg` only models integer
      scalars, `ptr` models `reg` and pointer addresses, and `mem`
      models `ptr` and memory contents.

3. `sea smt file.c -o file.smt2`: Generates CHC in SMT-LIB2 format. Is
   an alias for `sea fe` followed by `sea horn`. The command `sea pf`
   is an alias for `sea smt --prove`.

4.  `sea clp file.c -o file.clp`: Generates CHC in CLP format.

5. `sea lfe file.c -o file.ll` : runs the legacy front-end

To see all the commands, type `sea --help`. To see options for each
individual command CMD (e.g, `horn`), type `sea CMD --help` (e.g.,
`sea horn --help`).

## Annotating C programs ##

This is an example of a C program annotated with a safety property:
``` c
    /* verification command: sea pf --horn-stats test.c */
    #include "seahorn/seahorn.h"
    extern int nd();

    int main(void){
      int k=1;
      int i=1;
      int j=0;
      int n = nd();
      while(i<n) {
        j=0;
        while(j<i) {
          k += (i-j);
          j++;
        }
        i++;
      }
      sassert(k>=n);
    }

```
SeaHorn follows [SV-COMP](http://sv-comp.sosy-lab.org) convention of
encoding error locations by a call to the designated error function
`__VERIFIER_error()`. SeaHorn returns `unsat` when
`__VERIFIER_error()` is unreachable, and the program is considered
safe. SeaHorn returns `sat` when `__VERIFIER_error()` is reachable and
the program is unsafe. `sassert()` method is defined in
`seahorn/seahorn.h`.


# People #

* [Arie Gurfinkel](arieg.bitbucket.org)
* [Jorge Navas](http://jorgenavas.github.io/)
* [Temesghen Kahsai](http://www.lememta.info/)
