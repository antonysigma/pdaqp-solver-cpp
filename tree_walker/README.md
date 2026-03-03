# Pre-compiled binary decision tree walker implementations

This directory contains three alternative implementations of the PDA-QP
decision-tree traversal used to select the active region for the quadratic
program:

- **LUT** – loop over precomputed lookup tables:
- **SML** – compile-time nested switch statements implemented with state machine logic;
- **Heap** – binary tree stored in heap / Eytzinger layout.

Each implementation evaluates a sequence of hyperplane predicates `dot(n, p) <=
b` to traverse a pre-computed binary decision tree, and identify the active set
for solving the quadratic programming problem.

The implementations share the same decision function
`isInsideHalfplane(Parameter) -> bool` but differ in how the tree traversal
logic is represented in code and memory.

## Algorithm design trade-off study

## Test environment

- Target hardware: Atmel328p (Harvard architecture, 8-bit ALU with 16-bit flash ROM);
- Problem size: `n_parameter = 2`; `n_solution = 3`; `n_tree_nodes = 5`;
- Matrix-vector arithmetic: fixed-point decimals using a mix of Q2.14 and Q8.24.

## Common observations

Across all implementations, `applyFeedbackFn<>` dominates the firmware footprint
(716 Byte, roughly ~70–85% of the `pdaqpSolver()` function). The matrix and
vector coefficients used in the affine feedback laws are fully **inlined as
immediate operands (IMM)** inside the generated instructions. This layout is
particularly suitable for Harvard-architecture microcontrollers because it
avoids both SRAM traffic and explicit flash-to-register loads for coefficient
tables.

Differences between implementations are therefore driven primarily by
`treeWalker()` structure and the representation of the decision tree data.

### LUT tree walker

It generates small tables `hp_list` and `jump_list` in the `.rodata` section
(~20 byte) with moderate `.text` footprint. Because AVR cannot directly load
data from flash through normal pointers, lookup tables in `.rodata` may require
additional handling (e.g., `pgm_read_u16`) in production firmware. On
older-generation of AVR chipset (e.g. Atmel828p), the `.rodata` may be copied
into the sRAM at device boot, reducing the available free RAM.

```
$ ninja && bloaty -s vm -d symbols -n 0 ./main --source-filter=pdaqp
    FILE SIZE        VM SIZE
 --------------  --------------
  78.8%  5.82Ki  83.3%     716    (anonymous namespace)::pdaqp_solver_internal::applyFeedbackFn<>()
  18.9%  1.40Ki  14.0%     120    pdaqp_solver::treeWalker()
   0.6%      45   1.2%      10    pdaqp_hp_list
   0.6%      47   1.2%      10    pdaqp_jump_list
   1.1%      83   0.5%       4    (anonymous namespace)::pdaqp_solver_internal::hyperplane_fn_list
 100.0%  7.39Ki 100.0%     860    TOTAL
```

### Heap/Eytzinger walker

The heap representation has a slightly smaller flash memory footprint (~ 12
Byte) than the LUT tree walker. The traversal algorithm uses index arithmetic
(`i -> 2 * i + (0 or 1)`) and produces the **smallest traversal code** among all
implementations (~110 Byte of `.text`). Similar to the LUT walker, the halfspace
vector coefficients are completely inlined into the
`isInsideHalfplane<u16>(Parameter) -> bool` functions as immediate operands,
eliminating the need for separate coefficient tables.

```
    FILE SIZE        VM SIZE
 --------------  --------------
  75.5%  5.82Ki  85.0%     716    (anonymous namespace)::pdaqp_solver_internal::applyFeedbackFn<>()
  22.5%  1.73Ki  13.1%     110    pdaqp_solver::treeWalker()
   1.0%      76   1.4%      12    (anonymous namespace)::pdaqp_solver_internal::tree
   1.1%      83   0.5%       4    (anonymous namespace)::pdaqp_solver_internal::hyperplane_fn_list
 100.0%  7.71Ki 100.0%     842    TOTAL
```

### SML (aka nested switch statement) walker

The entire decision tree is expanded into explicit branch instructions inside
`treeWalker(Parameter) -> FeedbackID` using nested `if / switch` logic. Because
the traversal structure is directly encoded in the control flow (`brne`, `rjmp`,
etc), the lookup tables `jump_list` and `hp_list` disappear entirely. Similarly,
the halfspace/halfplane decision logic is completely inlined to every branch of
the binary tree, eliminating the function stack consumption. However, this
approach significantly increases the instruction footprint (~334 Byte of flash).

```
    FILE SIZE        VM SIZE
 --------------  --------------
  41.3%  5.82Ki  68.2%     716    (anonymous namespace)::pdaqp_solver_internal::applyFeedbackFn<>()
  58.7%  8.26Ki  31.8%     334    pdaqp_solver::treeWalker()
 100.0%  14.1Ki 100.0%  1.03Ki    TOTAL
```