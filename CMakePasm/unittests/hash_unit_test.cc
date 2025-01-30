#include <gtest/gtest.h>
#include "unit_test.h"

namespace hash_test
{
    static void hash_unit_test_method_initialize();
    static void hash_unit_test_method_cleanup();

    static void hash_unit_test_method_initialize()
    {
        initialize();
    }

    static void hash_unit_test_method_cleanup()
    {
        destroy();
    }

    TEST(hash_unit_test, hash_test)
    {
        const auto hash_value = hash_function(__FILE__);
        EXPECT_NE(0U, hash_value);

        const auto hash_value2 = hash_function(__FILE__);
        EXPECT_EQ(hash_value, hash_value2);
    }

    TEST(hash_unit_test, hash_pointer_function_test)
    {
        int dummy = 0x4545;

        const auto hash_value = hash_pointer_function(&dummy);
        EXPECT_NE(0U, hash_value);

        const auto hash_value2 = hash_pointer_function(&dummy);
        EXPECT_EQ(hash_value, hash_value2);
    }
}