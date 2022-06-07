# unit

[![Build](https://github.com/eliasku/unit/actions/workflows/build.yml/badge.svg)](https://github.com/eliasku/unit/actions/workflows/build.yml)
[![codecov](https://codecov.io/gh/eliasku/unit/branch/master/graph/badge.svg?token=NFTrtCHQ2r)](https://codecov.io/gh/eliasku/unit)

## 🥼 Tiny unit testing library for C language 🥼

[![Standard](https://img.shields.io/badge/C-11-pink.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Download](https://img.shields.io/badge/Download%20%20-unit.h-lightgreen.svg)](https://raw.githubusercontent.com/eliasku/unit/master/include/unit.h)
[![Documentation](https://img.shields.io/badge/docs-latest-white)](http://unit.rtfd.io/)
[![Twitter](https://img.shields.io/twitter/follow/eliaskuvoice.svg?style=flat&label=Follow&logoColor=white&color=1da1f2&logo=twitter)](https://twitter.com/eliaskuvoice)
[![Try it online](https://img.shields.io/badge/Try%20it-online-orange.svg)](https://godbolt.org/z/17vjqsnca)

![output](docs/output.gif)

## Example

Compile executable with `-D UNIT_TESTING` to enable tests

```c
#define UNIT_MAIN
#include <unit.h>

SUITE( suite name ) {
  DESCRIBE( subcase name ) {
    IT("is test behaviour") {
      WARN("use WARN_* to print failed assumptions");
      CHECK("use CHECK_* to continue execute assertions on fail");
      REQUIRE("use REQUIRE_* to skip next assertions on fail");
    }
  }
}

```

## Command-line options

- `--version`, `-v`: Prints the version of `unit` library
- `--help`, `-h`: Prints usage help message
- `--list`, `-l`: Prints all available tests
- `--animate`, `-a`: Simulate waits for printing messages, just for making fancy printing animation
- `--ascii`: Don't use colors and fancy unicode symbols in the output
- `--short-filenames`, `-S`: Use only basename for displaying file-pos information
- `--quiet`, `-q`: Disables all output
- `-r=xml`: Special switch prints XML report in DocTest-friendly format (for CLion test run configuration)

## Features and design goals

### ✓ Main focus and features

- Written in Pure C: only standard `libc` is used
- Simplicity and tiny build-size
- No dynamic memory allocations: only static memory is used for reporting test running infrastructure.
- Single-header library: easy to integrate
- Embedded runner & pretty reporter: build self-executable test
- Disable test code: allow you to write tests for your private implementation right at the end of `impl.c` file
- Cross-platform: should work for Linux / macOS / Windows / WebAssembly

### ✕ What you won't find here

- Cross-compiler support: no `MSVC` support, only `clang` is tested
- Multithreading and parallel test running
- Tricky test matchers design
- Fixtures, `before` / `after` or mocking
- Crash tests and signal interception
- Fuzz testing

> In any case, if you have a desire, you can support and contribute! Feel free to ask me any **feature** you need. 
> **Thank you for your interest!** 
