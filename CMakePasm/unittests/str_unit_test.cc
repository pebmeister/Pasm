#include <gtest/gtest.h>
#include "unit_test.h"

namespace str_test
{
#pragma warning(disable : 4996 4090)

    static void str_unit_test_method_initialize();
    static void str_unit_test_method_cleanup();

    static void str_unit_test_method_initialize()
    {
        initialize();
    }

    static void str_unit_test_method_cleanup()
    {
        destroy();
    }

    char* message_ = new char[2048];

    typedef struct test_case {
        const char* test_str;
        const char* expected_str;
    } test_case, * test_case_ptr;

    const int num_cases_ = 11;

    test_case_ptr test_cases_ = new test_case[num_cases_]
    {
        test_case
        {
            "",
            ""
        },
        test_case
        {
            R"(cat\a)",
            "cat\a"
        },
        test_case
        {
            R"(\bcat)",
            "\bcat"
        },
        test_case
        {
            R"(cat\f)",
            "cat\f"
        },
        test_case
        {
            R"(cat\v)",
            "cat\v"
        },
        test_case
        {
            R"(\rcat)",
            "\rcat"
        },
        test_case
        {
            "cat\\n",
            "cat\n"
        },
        test_case
        {
            R"(\tcat)",
            "\tcat"
        },
        test_case
        {
            R"(cat \x5F)",
            "cat _"
        },
        test_case
        {
            R"(\'cat\')",
            "'cat'"
        },
        test_case
        {
            R"("cat")",
            "\"cat\""
        }
    };

    TEST(str_unit_test, sanitize_string_unit_test)
    {
        str_unit_test_method_initialize();

        for (auto i = 0; i < num_cases_; ++i) {
            const auto actual = sanitize_string(const_cast<char*>(test_cases_[i].test_str));

            EXPECT_STREQ(test_cases_[i].expected_str, actual);
        }

        str_unit_test_method_cleanup();
    }
}