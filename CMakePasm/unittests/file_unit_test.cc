#include <gtest/gtest.h>
#include "unit_test.h"

namespace file_test
{
    static void file_unit_test_method_initialize()
    {
        initialize();

        yyin = nullptr;
        directories = (char*)"";  // NOLINT(clang-diagnostic-writable-strings, modernize-raw-string-literal)
    }

    static void file_unit_test_method_cleanup()
    {
        if (yyin != nullptr) {
            fclose(yyin);
            yyin = nullptr;
        }

        current_file_name = nullptr;
        directories = (char*)"";  // NOLINT(clang-diagnostic-writable-strings, modernize-raw-string-literal)
        destroy();
    }

    TEST(file_unit_test, open_file_test)
    {
        file_unit_test_method_initialize();

        const auto file_text =
            "	.ORG $1000\n"
            "	nop\n"
            "\n";

        const auto* outfile_name = "open_file_test.a";
        auto* outfile = open_file(outfile_name, "w");
        EXPECT_NOT_NULL(outfile);
        fwrite(file_text, 1, strlen(file_text), outfile);
        fclose(outfile);

        yyin = open_file(outfile_name, "r");
        EXPECT_NOT_NULL(yyin);
        fclose(yyin);
        yyin = nullptr;

        remove(outfile_name);

        file_unit_test_method_cleanup();
    }

    TEST(file_unit_test, include_file_test)
    {
        file_unit_test_method_initialize();

        const auto file_text =
            "	.ORG $1000\n"
            "	nop\n"
            "\n";

        const auto include_file_text =
            "	.ORG $2000\n"
            "	nop\n"
            "\n";

        char* outfile_name = (char*)"open_file_test.a";

        char* include_file_name = (char*)"open_includefile_test.a";
        auto* outfile = open_file(include_file_name, "w");
        EXPECT_NOT_NULL(outfile);
        fwrite(include_file_text, 1, strlen(file_text), outfile);
        fclose(outfile);

        current_file_name = outfile_name;
        outfile = open_file(current_file_name, "w");
        EXPECT_NOT_NULL(outfile);
        fwrite(file_text, 1, strlen(file_text), outfile);
        fclose(outfile);

        yyin = open_file(current_file_name, "r");
        EXPECT_NOT_NULL(yyin);

        if (yyin != nullptr) {
            fseek(yyin, 0, SEEK_SET);  // NOLINT(cert-err33-c)

            // reset parser
            yyrestart(yyin);
        }

        const int file = open_include_file(include_file_name);
        EXPECT_EQ(0, file);
        fclose(yyin);

        remove(output_file_name);
        remove(include_file_name);

        yyin = nullptr;

        file_unit_test_method_cleanup();
    }

    TEST(file_unit_test, read_file_lines_test)
    {
        file_unit_test_method_initialize();

        const auto outfile_name = "read_file_lines_test.a";
        const auto outfile = open_file(outfile_name, "w");
        EXPECT_NOT_NULL(outfile);

        for (auto i = 1; i <= 1000; ++i) {
            fprintf(outfile, "READ LINE %d\n", i);
        }
        fclose(outfile);

        file_line_node* file_lines = read_file_lines(outfile_name);
        EXPECT_NOT_NULL(file_lines);
        const auto* temp = file_lines;
        for (int line = 1; line <= 1000; ++line, temp = temp->next) {
            constexpr  int maximum_line_length = 1024;
            char buffer[maximum_line_length] = { 0 };
            sprintf(buffer, "READ LINE %d\n", line);
            auto result = strcmp(buffer, temp->line_content);
            EXPECT_EQ(0, result);
        }
        free_file_lines(file_lines);

        remove(outfile_name);

        file_unit_test_method_cleanup();
    }

    TEST(file_unit_test, free_file_lines)
    {
        file_unit_test_method_initialize();

        const auto outfile_name = "free_file_lines_test.a";
        const auto outfile = open_file(outfile_name, "w");
        EXPECT_NOT_NULL(outfile);

        for (auto i = 1; i <= 1000; ++i) {
            fprintf(outfile, "READ LINE %d\n", i);
        }
        fclose(outfile);

        file_line_node* file_lines = read_file_lines(outfile_name);
        EXPECT_NOT_NULL(file_lines);
        auto* temp = file_lines;
        for (int line = 1; line <= 1000; ++line, temp = temp->next) {
            constexpr  int maximum_line_length = 1024;
            char buffer[maximum_line_length] = { 0 };
            sprintf(buffer, "READ LINE %d\n", line);
            auto result = strcmp(buffer, temp->line_content);
            EXPECT_EQ(0, result);
        }
        free_file_lines(file_lines);

        remove(outfile_name);
        remove(outfile_name);

        file_unit_test_method_cleanup();
    }
}