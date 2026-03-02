# PDAQP Solver in C++20

This repository presents a modern C++20 implementation targeting:

- 16-bit microcontrollers for small problem sizes, and
- Vectorized NEON/AVX-class CPUs for larger problem sizes.

PDA-QP Solver is a high-performance quadratic programming solver based on a
pre-computed binary decision tree. It is particularly well suited for hard
real-time control systems, where execution time must be bounded, and algorithm
convergence must occur with an engineer-specified deadline. Unlike iterative
methods that rely on runtime convergence checks, PDA-QP provides **deterministic
execution behavior by design, offering algorithm-level guarantees on CPU/MCU
clock cycles counts and timing**.

The implementation emphasizes compile-time generation of efficient control code
and memory-aligned data encoding in the firmware/executable, enabling bare-metal
deployments without dependence on heavyweight runtime libraries. The goal is to
demonstrate how modern C++ can deliver hardware-accelerated firmware, while
remaining portable across MCU and CPU architectures.

For citations, refer to the original author of the algorithm: https://github.com/darnstrom/pdaqp

## Motivation

Modern C++ (C++20) gives firmware developers many of the same "design knobs"
that FPGA/ASIC developers expect in RTL and SystemVerilog: aggressive
compile-time composition, static dispatch, and data-layout control. The goal of
this project is to demonstrate that you can use all compute capabilities of
MCU/CPU targets (including *single instruction multiple data* SIMD where
available) while keeping the deployment baremetal friendly, without the need of
heavy runtime libraries.

## Target architectures

- Microchip AVR / ATmega (8-bit MCUs)
- ARM 32-bit MCUs (e.g., Teensy-class), including NEON where available
- Workstation-grade CPUs (e.g., Ryzen 5-class), including AVX where available

## Folder structure

This repository is organized as a small set of components (subject to change as
the project evolves):

- `tree_walker/`: decision-tree traversal logic, branch/jump table code
  generator, and custom SIMD-optimized memory layout;
- `apply_feedback/`: affine transformation operators, custom SIMD-optimized
  memory layout for the control feedback step;
- `common/`: shared types and utilities;
- `examples/`: examples control problems in various problem sizes;
- `subprojects/`: third-party dependencies for compile-time code generation.

## Quick start

On a Linux build machine, install the Astral UV: the python-pip accelerator.
Then, install the build system:

```bash
cd pdaqp-solver-cpp/
uv venv --python=3.12
source .venv/bin/activate
uv pip install -U meson ninja
```

Next, compile the example program:

```bash
meson setup build/
ninja -C build/
```

## Usage

Assuming your design workflow is based on CVXPyGen: code generation from
constrained quadratic programming problem formulations, the tool should generate
baremetal code in C99 language. Copy the values from `pdaqp.c` to
`examples/mpc_re/problem-def.hpp` and the problem sizes to
`examples/mpc_re/constants.hpp`. And then rebuild the example program.

For a custom folder `examples/user/` containing the same files, configure the build system to use it by:
```bash
cd pdaqp-solver-cpp/
meson setup --wipe -Dname=user build/
```

And the rebuild the example program again.

To test the example program:
```bash
cd pdaqp-solver-cpp/
./build/pdaqp-solver ${ascii-enocded parameter array in little endian notation}
```

## Detailed design document

(To be determined)

- [ ] Compile-time binary tree decoding via C++20 `consteval`;

- [ ] Fixed-point decimal maths via C++20 concepts;

- [ ] Three implementation options for the binary decision tree: Lookup jump
  table (the default implementation from
  [CVXPyGen](https://github.com/cvxgrp/cvxpygen)), the [Eytzinger-style cache
  aware search](https://arxiv.org/abs/1509.05053), and the [State Machine
  Logic](https://github.com/boost-ext/sml)

- [ ] Two implementation options for the affine transform step: Function pointer
  table with hardcoded Scale matrices and offset vectors; inlined *fused
  multiply add* operations via compiler-optimized tail function calls.
