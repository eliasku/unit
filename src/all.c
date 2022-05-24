/**
 * unit.h - v${VERSION} - ${DESCRIPTION} - https://github.com/eliasku/unit
**/

#include "unit.h"

#if defined(UNIT_IMPL) && !defined(UNIT_C_IMPLEMENTED) && defined(UNIT_TESTING)
#define UNIT_C_IMPLEMENTED

#include "unit.c"

#endif // => UNIT_C_IMPLEMENTED
