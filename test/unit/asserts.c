// Проверяем, что компиляция заголовка возможна в более чем одной единице сборки (Translation Unit)
#include <unit.h>
#include <stdlib.h>

SUITE(asserts) {
    IT("evaluated only once!") {
        int a = 5;

        REQUIRE_EQ(++a, 6);
        REQUIRE(++a == 7);
        CHECK_EQ(++a, 8);
        CHECK(++a == 9);
        WARN_EQ(++a, 10);
        WARN(++a == 11);

        // дальше мы пропускаем проверки, они будут помечены в отчёте как пропущенные,
        // но они не должны выполнять проверяемые выражения внутри, значит переменная не должна больше изменяться
        SKIP();
        WARN_EQ(++a, 12);
        WARN(++a == 13);
        CHECK_EQ(++a, 14);
        CHECK(++a == 15);
        REQUIRE_EQ(++a, 16);
        REQUIRE(++a == 17);

        // этот код выполняется в любом случае,
        if (a != 11) exit(EXIT_FAILURE);
    }

    IT("has int support") {
        int b = -10;
        int zero = 0;

        WARN(b);
        WARN_FALSE(zero);
        WARN_NE(b, 99);
        WARN_EQ(b, -10);
        WARN_GE(b, -10);
        WARN_GT(b, -11);
        WARN_LE(b, -10);
        WARN_LT(b, -9);

        CHECK(b);
        CHECK_FALSE(zero);
        CHECK_NE(b, 99);
        CHECK_EQ(b, -10);
        CHECK_GE(b, -10);
        CHECK_GT(b, -11);
        CHECK_LE(b, -10);
        CHECK_LT(b, -9);

        REQUIRE(b);
        REQUIRE_FALSE(zero);
        REQUIRE_NE(b, 99);
        REQUIRE_EQ(b, -10);
        REQUIRE_GE(b, -10);
        REQUIRE_GT(b, -11);
        REQUIRE_LE(b, -10);
        REQUIRE_LT(b, -9);
    }

    IT("has uint support") {
        unsigned int b = 10;
        unsigned int zero = 0;

        WARN(b);
        WARN_FALSE(zero);
        WARN_NE(b, 99u);
        WARN_EQ(b, 10u);
        WARN_GE(b, 10u);
        WARN_GT(b, 9u);
        WARN_LE(b, 10u);
        WARN_LT(b, 11u);

        CHECK(b);
        CHECK_FALSE(zero);
        CHECK_NE(b, 99u);
        CHECK_EQ(b, 10u);
        CHECK_GE(b, 10u);
        CHECK_GT(b, 9u);
        CHECK_LE(b, 10u);
        CHECK_LT(b, 11u);

        REQUIRE(b);
        REQUIRE_FALSE(zero);
        REQUIRE_NE(b, 99u);
        REQUIRE_EQ(b, 10u);
        REQUIRE_GE(b, 10u);
        REQUIRE_GT(b, 9u);
        REQUIRE_LE(b, 10u);
        REQUIRE_LT(b, 11u);
    }

    IT("has float support") {
        double b = 10.01;
        double zero = 0.0;

        WARN(b);
        WARN_FALSE(zero);
        WARN_NE(b, 99.0);
        WARN_EQ(b, 10.01);
        WARN_GE(b, 10.01);
        WARN_GT(b, 9.0);
        WARN_LE(b, 10.01);
        WARN_LT(b, 11.0);

        CHECK(b == 10.01);
        CHECK_FALSE(b != 10.01);
        CHECK_NE(b, 99.0);
        CHECK_EQ(b, 10.01);
        CHECK_GE(b, 10.01);
        CHECK_GT(b, 9.0);
        CHECK_LE(b, 10.01);
        CHECK_LT(b, 11.0);

        REQUIRE(b == 10.01);
        REQUIRE_FALSE(b != 10.01);
        REQUIRE_NE(b, 99.0);
        REQUIRE_EQ(b, 10.01);
        REQUIRE_GE(b, 10.01);
        REQUIRE_GT(b, 9.0);
        REQUIRE_LE(b, 10.01);
        REQUIRE_LT(b, 11.0);
    }

    IT("has c-string support") {
        WARN("is not empty");
        WARN_FALSE((const char*) NULL);
        WARN_FALSE("");
        WARN_NE("dog", "cat");
        WARN_EQ("bird", "bird");
        WARN_GE("bird", "alpha");
        WARN_GT("bird", "alpha");
        WARN_LE("alpha", "bird");
        WARN_LT("alpha", "bird");

        CHECK("is not empty");
        CHECK_FALSE((const char*) NULL);
        CHECK_FALSE("");
        CHECK_NE("dog", "cat");
        CHECK_EQ("bird", "bird");
        CHECK_GE("bird", "alpha");
        CHECK_GT("bird", "alpha");
        CHECK_LE("alpha", "bird");
        CHECK_LT("alpha", "bird");

        REQUIRE("is not empty");
        REQUIRE_FALSE((const char*) NULL);
        REQUIRE_FALSE("");
        REQUIRE_NE("dog", "cat");
        REQUIRE_EQ("bird", "bird");
        REQUIRE_GE("bird", "alpha");
        REQUIRE_GT("bird", "alpha");
        REQUIRE_LE("alpha", "bird");
        REQUIRE_LT("alpha", "bird");
    }

    IT("accept NULL c-string as empty string") {
        REQUIRE_FALSE((const char*) NULL);
        REQUIRE_GT("alpha", (const char*) NULL);
        REQUIRE_LT((const char*) NULL, "omega");
        REQUIRE_EQ(NULL, "");
        REQUIRE_NE(NULL, "beta");
        REQUIRE_LE(NULL, "");
        REQUIRE_GE(NULL, "");
    }

    IT("compares pointer type") {
        int v1 = 1; // 0
        int v2 = 2; // 1
        const void* ptr1 = &v1;
        const void* ptr2 = &v2;
        REQUIRE(v1);
        REQUIRE(v2);
        REQUIRE(ptr1);
        REQUIRE(ptr2);
        REQUIRE_LT(ptr2, ptr1);
        REQUIRE_GT(ptr1, ptr2);
        REQUIRE_LE(NULL, ptr2);
        REQUIRE_GE(ptr2, NULL);
    }

    IT("has void* support") {
        const void* zero = NULL;
        const void* ptr = &zero;
        REQUIRE_FALSE(zero);
        REQUIRE(ptr);
        REQUIRE_EQ(zero, NULL);
        REQUIRE_NE(ptr, zero);
    }

    IT("fail cases", .failing=1) {
        const void* zero = NULL;
        const char* str = NULL;
        unsigned u = 2;
        int s = -2;
        double f = 0.0;

        CHECK(zero);
        CHECK_EQ(zero, (const void*) &str);
        CHECK(str);
        CHECK_EQ(str, "string");
        CHECK_FALSE(u);
        CHECK_LE(u, 0u);
        CHECK_FALSE(s);
        CHECK_GT(s, 0);
        CHECK(f);
        CHECK_GT(f, 0.0);
    }
}
