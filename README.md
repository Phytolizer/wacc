# wacc - What A C Compiler

wacc is a compiler for a subset of the C programming language. It is written in C and compiles to x86_64 assembly.

# Building

wacc builds with CMake. To build, run the following commands:

```bash
cmake --preset dist  # or --preset dev for a debug build
cmake --build --preset dist  # or --preset dev
```

# Testing

wacc has a test suite. To run the test suite, run the following commands:

```bash
cd out/build/dist  # or out/build/dev
ctest
```
