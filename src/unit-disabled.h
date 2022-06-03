
#define UNIT__NOOP (void)(0)
#define UNIT__CONCAT_(a, b) a ## b
#define UNIT__CONCAT(a, b) UNIT__CONCAT_(a, b)
#define UNIT_SUITE(Name, ...) __attribute__((unused)) static void UNIT__CONCAT(unit__, __COUNTER__)(void)
#define UNIT_DESCRIBE(Name, ...) while(0)
#define UNIT_TEST(Description, ...) while(0)

#define UNIT_ECHO(...) UNIT__NOOP

#define UNIT_WARN(x, ...) UNIT__NOOP
#define UNIT_WARN_FALSE(x, ...) UNIT__NOOP
#define UNIT_WARN_EQ(a, b, ...) UNIT__NOOP
#define UNIT_WARN_NE(a, b, ...) UNIT__NOOP
#define UNIT_WARN_GT(a, b, ...) UNIT__NOOP
#define UNIT_WARN_GE(a, b, ...) UNIT__NOOP
#define UNIT_WARN_LT(a, b, ...) UNIT__NOOP
#define UNIT_WARN_LE(a, b, ...) UNIT__NOOP

#define UNIT_CHECK(x, ...) UNIT__NOOP
#define UNIT_CHECK_FALSE(x, ...) UNIT__NOOP
#define UNIT_CHECK_EQ(a, b, ...) UNIT__NOOP
#define UNIT_CHECK_NE(a, b, ...) UNIT__NOOP
#define UNIT_CHECK_GT(a, b, ...) UNIT__NOOP
#define UNIT_CHECK_GE(a, b, ...) UNIT__NOOP
#define UNIT_CHECK_LT(a, b, ...) UNIT__NOOP
#define UNIT_CHECK_LE(a, b, ...) UNIT__NOOP

#define UNIT_REQUIRE(x, ...) UNIT__NOOP
#define UNIT_REQUIRE_FALSE(x, ...) UNIT__NOOP
#define UNIT_REQUIRE_EQ(a, b, ...) UNIT__NOOP
#define UNIT_REQUIRE_NE(a, b, ...) UNIT__NOOP
#define UNIT_REQUIRE_GT(a, b, ...) UNIT__NOOP
#define UNIT_REQUIRE_GE(a, b, ...) UNIT__NOOP
#define UNIT_REQUIRE_LT(a, b, ...) UNIT__NOOP
#define UNIT_REQUIRE_LE(a, b, ...) UNIT__NOOP

#define UNIT_SKIP(...) UNIT__NOOP

#define unit_main(...) (0)
