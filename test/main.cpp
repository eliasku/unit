#include <unit.h>
#include <cstring>

suite(unit) {
    it("should compile as c++ source code") {
        require(strstr(__PRETTY_FUNCTION__, __FUNCTION__));
    }
}