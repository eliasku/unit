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

### Verbose Mode

```c
#define UNIT_VERBOSE
/*^^^^^^^^^^^^^^^^*/
#define UNIT_MAIN
#include <unit.h>
```

![verbose](docs/verbose.png)
