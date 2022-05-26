/**
 * unit.h - v${VERSION} - ${DESCRIPTION} - https://github.com/eliasku/unit
 *
 * Minimal example. Compile executable with `-D UNIT_TESTING` to enable tests.
 *
 * ```c
 * #define UNIT_IMPL
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
 **/

/**
 * Enable unit testing
 **/
// #define UNIT_TESTING

/**
 * Disable colorful output
 */
// #define UNIT_NO_COLORS

/**
 * Disable default `main` generation
 */
// #define UNIT_NO_MAIN

/**
 * Disable time measurements
 */
// #define UNIT_NO_TIME

/**
 * Add run details to the output (echo, assertions)
 */
// #define UNIT_VERBOSE

#include "unit-def.h"

#if defined(UNIT_IMPL) && !defined(UNIT_C_IMPLEMENTED) && defined(UNIT_TESTING)
#define UNIT_C_IMPLEMENTED

#include "unit.c"

#endif // => UNIT_C_IMPLEMENTED
