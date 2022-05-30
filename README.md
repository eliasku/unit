# unit

[![Standard](https://img.shields.io/badge/C-11/14/17-pink.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Download](https://img.shields.io/badge/Download%20%20-unit.h-lightgreen.svg)](https://raw.githubusercontent.com/eliasku/unit/master/include/unit.h)
[![Documentation](https://img.shields.io/badge/docs-latest-white)](http://unit.rtfd.io/)
[![Twitter](https://img.shields.io/twitter/follow/eliaskuvoice.svg?style=flat&label=Follow&logoColor=white&color=1da1f2&logo=twitter)](https://twitter.com/eliaskuvoice)
[![Try it online](https://img.shields.io/badge/Try%20it-online-orange.svg)](https://godbolt.org/z/ajG6v7ecK)

## 🥼 Tiny unit testing library for C language 🥼

[![Build](https://github.com/eliasku/unit/actions/workflows/build.yml/badge.svg)](https://github.com/eliasku/unit/actions/workflows/build.yml)
[![codecov](https://codecov.io/gh/eliasku/unit/branch/master/graph/badge.svg?token=NFTrtCHQ2r)](https://codecov.io/gh/eliasku/unit)

![screencast](docs/screencast.gif)

## Example

Compile executable with `-D UNIT_TESTING` to enable tests

```c
#define UNIT_MAIN
#include <unit.h>

suite( you_should_define_the_suite_name_here ) {
  describe( optionally_add_the_subject ) {
    it( "describe behaviour" ) {
      check("use warn / check / require functions");
    }
  }
}

```

![output](docs/output.png)

## Features and design goals

### ✓ Main focus and features
- Written in Pure C: only standard `libc` usage
- Simplicity and tiny build-size
- No dynamic memory allocations: only static memory is used for reporting test running infrastructure.
- Single-header library: easy to integrate
- Embedded runner & pretty reporter: build self-executable test
- Abi[lity to mute test code: allow you to write tests for your private implementation right at the end of `impl.c` file
- Cross-platform: should work for Linux / macOS / Windows 

### ✕ What you won't find here
- Cross-compiler support: no `MSVC` support, only `clang` is tested
- Multithreading and parallel test running
- Tricky test matchers design
- Fixtures, `before` / `after` or mocking
- Crash tests and signal interception
- Fuzz testing

> In any case, if you have a desire, you can support and contribute! Feel free to ask me any **feature** you need **Thank you for your interest!** 
