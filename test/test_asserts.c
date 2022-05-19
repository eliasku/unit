#include <unit.h>

suite(asserts) {
    static int a = 5;

    it("evaluated only once!") {
        require_eq(++a, 6);
        require(++a == 7);
        check_eq(++a, 8);
        check(++a == 9);

        // дальше мы пропускаем проверки, они будут помечены в отчёте как пропущенные,
        // но они не должны выполнять проверяемые выражения внутри, значит переменная не должна больше изменяться
        skip();
        require_eq(++a, 10);
        require(++a == 11);
        check_eq(++a, 12);
        check(++a == 13);

        // этот код выполняется в любом случае,
        if (a != 9) exit(EXIT_FAILURE);
    }

    it("has compares") {
        int b = 10;
        require_ne(b, 99);
        require_eq(b, 10);
        require_ge(b, 10);
        require_gt(b, 9);
        require_le(b, 10);
        require_lt(b, 11);
        check_ne(b, 99);
        check_eq(b, 10);
        check_ge(b, 10);
        check_gt(b, 9);
        check_le(b, 10);
        check_lt(b, 11);
    }
}
