#include <gtest/gtest.h>
#include "unit_test.h"

namespace error_test
{
    static void error_unit_test_method_initialize()
    {
        initialize();
    }

    static void error_unit_test_method_cleanup()
    {
        destroy();
    }

    TEST(error_unit_test, error_test)
    {
        error_unit_test_method_initialize();

        for (int error_num = error_start + 1; error_num < error_max; ++error_num) {
            error(error_num);
        }

        error_unit_test_method_cleanup();
    }

    TEST(error_unit_test, error_test2)
    {
        error_unit_test_method_initialize();

        for (int error_num = error_start + 1; error_num < error_max; ++error_num) {
            error2(error_num, __FILE__);
        }

        error_unit_test_method_cleanup();
    }
}