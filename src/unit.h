/**
 * unit.h - v${VERSION} - ${DESCRIPTION} - https://github.com/eliasku/unit
 *
 * Minimal example. Compile executable with `-D UNIT_TESTING` to enable tests.
 *
 * ```c
 * #define UNIT_MAIN
 * #include "unit.h"
 *
 * suite( you_should_define_the_suite_name_here ) {
 *   describe( optionally_add_the_subject ) {
 *     it( "describe the test behaviour" ) {
 *       check("use warn / check / require functions");
 *     }
 *   }
 * }
 * ```
 *
 * ## Enable Tests
 *
 * By default all test code is stripped away, to enable test-code you should pass `-D UNIT_TESTING` and build executable
 *
 * ## Default configuration
 *
 * In any translation unit you need to `#define UNIT_MAIN` before `#include <unit.h>` to implement the library and
 * generate default main entry-point to run all tests.
 *
 * ## Custom `main()`
 *
 * If you need just to implement library, you `#define UNIT_IMPLEMENT` before `include <unit.h>` in any single translation unit
 *
 **/

/**
 * Declare default main arguments
 */
// #define UNIT_DEFAULT_ARGS "--no-colors", "--trace"

#include "unit-def.h"

#ifdef UNIT_TESTING

#define UNIT_VERSION "${VERSION}"

/**
 * @deprecated use UNIT_IMPLEMENT or UNIT_MAIN options
 */
#ifdef UNIT_IMPL
#define UNIT_MAIN
#endif // UNIT_IMPL

#ifdef UNIT_MAIN
#define UNIT_IMPLEMENT
#endif // UNIT_MAIN

#ifdef UNIT_IMPLEMENT

#include "unit.c"

#endif // UNIT_IMPLEMENT

#endif // UNIT_TESTING
