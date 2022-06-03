#include <unit.h>
#include <cstring>

SUITE(unit.cpp) {
    IT("should compile as c++ source code") {
        REQUIRE(strstr(__PRETTY_FUNCTION__, __FUNCTION__));
    }
}
