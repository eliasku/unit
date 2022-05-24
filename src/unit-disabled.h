
#define UNIT__CONCAT_(a, b) a ## b
#define UNIT__CONCAT(a, b) UNIT__CONCAT_(a, b)
#define UNIT_SUITE(Name, ...) __attribute__((unused)) static void UNIT__CONCAT(Name, __COUNTER__)(void)
#define UNIT_DESCRIBE(Name, ...) while(0)
#define UNIT_IT(Description, ...) while(0)

#define UNIT_ECHO(...)

#define UNIT_WARN(x, ...)
#define UNIT_WARN_FALSE(x, ...)
#define UNIT_WARN_EQ(a, b, ...)
#define UNIT_WARN_NE(a, b, ...)
#define UNIT_WARN_GT(a, b, ...)
#define UNIT_WARN_GE(a, b, ...)
#define UNIT_WARN_LT(a, b, ...)
#define UNIT_WARN_LE(a, b, ...)

#define UNIT_CHECK(x, ...)
#define UNIT_CHECK_FALSE(x, ...)
#define UNIT_CHECK_EQ(a, b, ...)
#define UNIT_CHECK_NE(a, b, ...)
#define UNIT_CHECK_GT(a, b, ...)
#define UNIT_CHECK_GE(a, b, ...)
#define UNIT_CHECK_LT(a, b, ...)
#define UNIT_CHECK_LE(a, b, ...)

#define UNIT_REQUIRE(x, ...)
#define UNIT_REQUIRE_FALSE(x, ...)
#define UNIT_REQUIRE_EQ(a, b, ...)
#define UNIT_REQUIRE_NE(a, b, ...)
#define UNIT_REQUIRE_GT(a, b, ...)
#define UNIT_REQUIRE_GE(a, b, ...)
#define UNIT_REQUIRE_LT(a, b, ...)
#define UNIT_REQUIRE_LE(a, b, ...)

#define UNIT_SKIP()
