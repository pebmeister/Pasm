#include <gtest/gtest.h>
#include "unit_test.h"


static void dictionary_unit_test_method_initialize();
static void dictionary_unit_test_method_cleanup();

static dictionary_ptr test_dict = nullptr;
static long sz = sizeof(char);

static void create_key(char* key, const int key_size, const char ch)
{
    memset(key, ch, key_size);
    key[key_size] = 0;
}

static void dictionary_unit_test_method_initialize()
{
    initialize();
    reset_lex();
    
    test_dict = dict_create(sz, 0);    
}

static void dictionary_unit_test_method_cleanup()
{
    destroy();
    dict_destroy(test_dict);
    test_dict = nullptr;
}

TEST(dictionary_unit_test, dict_create_test)
{
    dictionary_unit_test_method_initialize();
    
    EXPECT_NOT_NULL(test_dict);
    EXPECT_EQ(sz, test_dict->element_size);
    EXPECT_EQ(0L, test_dict->number_elements);
    
    dictionary_unit_test_method_cleanup();
}

TEST(dictionary_unit_test, dict_insert_test)
{
    dictionary_unit_test_method_initialize();
    
    EXPECT_NOT_NULL(test_dict);
    EXPECT_EQ(sz, test_dict->element_size);
    EXPECT_EQ(0L, test_dict->number_elements);

    // test insert
    constexpr int key_size = 10;
    char key[key_size + 1];

    long count = 0;
    for (char v = 'a'; v <= 'z'; v++)
    {
        create_key(key, key_size, v);
        dict_insert(&test_dict, key, &v);
        count++;
        EXPECT_EQ(count, test_dict->number_elements);
    }

    for (char v = 'a'; v <= 'z'; v++)
    {
        create_key(key, key_size, v);
        const char* found = static_cast<char*>(dict_search(test_dict, key));
        EXPECT_NOT_NULL(found);
        EXPECT_EQ(v, *found);
    }

    // test when dictionary is forced to grow
    const auto current_size = test_dict->size;

    // declaring output string stream
    std::ostringstream str_stream;
    for (count = 0; count < current_size + 10; ++count)
    {
        str_stream << count;
        auto str = str_stream.str();
        dict_insert(&test_dict, (void*)(str.c_str()), (void*)(str.c_str()));
        str.clear();
    }
    EXPECT_NE(current_size, test_dict->size);

    // check that it is not corrupt
    for (char v = 'a'; v <= 'z'; v++)
    {
        create_key(key, key_size, v);
        const char* found = static_cast<char*>(dict_search(test_dict, key));
        EXPECT_NOT_NULL(found);
        EXPECT_EQ(v, *found);
    }
    
    dictionary_unit_test_method_cleanup();
}

TEST(dictionary_unit_test, dict_search_test)
{
    dictionary_unit_test_method_initialize();
    
    EXPECT_NOT_NULL(test_dict);
    EXPECT_EQ(sz, test_dict->element_size);
    EXPECT_EQ(0L, test_dict->number_elements);

    constexpr int key_size = 10;
    char key[key_size + 1] = { 0 };

    for (char v = 'a'; v <= 'z'; v++)
    {
        create_key(key, key_size, v);
        dict_insert(&test_dict, key, &v);
    }

    for (char v = 'a'; v <= 'z'; v++)
    {
        create_key(key, key_size, v);
        const char* found = static_cast<char*>(dict_search(test_dict, key));
        EXPECT_NOT_NULL(found);
        EXPECT_EQ(v, *found);
    }

    for (char v = 'Z'; v >= 'A'; v--)
    {
        create_key(key, key_size, v);
        const char* found = static_cast<char*>(dict_search(test_dict, key));
        EXPECT_NOT_NULL(found);
        EXPECT_EQ(static_cast<char>(std::tolower(v)), *found);
    }

    for (char v = '0'; v <= '9'; v++)
    {
        create_key(key, key_size, v);
        const char* found = static_cast<char*>(dict_search(test_dict, key));
        EXPECT_EQ(found, nullptr);
    }
    dictionary_unit_test_method_cleanup();

}

TEST(dictionary_unit_test, dict_delete_test)
{
    dictionary_unit_test_method_initialize();

    EXPECT_NOT_NULL(test_dict);
    EXPECT_EQ(sz, test_dict->element_size);
    EXPECT_EQ(0L, test_dict->number_elements);

    constexpr int key_size = 10;
    char key[key_size + 1] = { 0 };

    char key_values['z' - 'a' + 1] = { 0 };

    for (char v = 'a'; v <= 'z'; v++)
    {
        key_values[v - 'a'] = v;
        create_key(key, key_size, v);
        dict_insert(&test_dict, key, &v);
    }

    int step = 12;
    long element_count = test_dict->number_elements;
    while (step > 0)
    {
        for (auto i = 0; i < 'z' - 'a' + 1; i += step)
        {
            bool element_delete = false;
            if (key_values[i] != 0)
            {
                create_key(key, key_size, key_values[i]);
                dict_delete(test_dict, key);
                key_values[i] = 0;
                element_count--;
                EXPECT_EQ(test_dict->number_elements, element_count);
                element_delete = true;
            }

            if (element_delete)
            {
                int j = 0;
                for (char v = 'a'; v <= 'z'; v++, j++)
                {
                    create_key(key, key_size, v);
                    const char* found = static_cast<char*>(dict_search(test_dict, key));

                    if (key_values[j] == 0)
                    {
                        EXPECT_EQ(found, nullptr);
                    }
                    else
                    {
                        EXPECT_EQ(key_values[j], *found);
                    }
                }
            }
        }
        step--;
    }

    for (char v = 'a'; v <= 'z'; v++)
    {
        key_values[v - 'a'] = v;
        create_key(key, key_size, v);
        dict_insert(&test_dict, key, &v);
    }

    for (char v = 'a'; v <= 'z'; v++)
    {
        create_key(key, key_size, v);
        const char* found = static_cast<char*>(dict_search(test_dict, key));
        EXPECT_NOT_NULL(found);
        EXPECT_EQ(v, *found);
    }
    
    dictionary_unit_test_method_cleanup();
}

TEST(dictionary_unit_test, dict_destroy_test)
{
    dictionary_unit_test_method_initialize();
    dict_destroy(test_dict);
    test_dict = nullptr;
    dictionary_unit_test_method_cleanup();
}

TEST(dictionary_unit_test, dump_dictionary_test)
{
    dictionary_unit_test_method_initialize();

    EXPECT_NOT_NULL(test_dict);
    EXPECT_EQ(sz, test_dict->element_size);
    EXPECT_EQ(0L, test_dict->number_elements);

    constexpr int key_size = 10;
    char key[key_size + 1] = { 0 };

    for (char v = 'a'; v <= 'z'; v++)
    {
        create_key(key, key_size, v);
        dict_insert(&test_dict, key, &v);
    }

    FILE* temp_file = open_file("dump_dictionary_test.txt", "w");

    dump_dictionary(test_dict, nullptr, temp_file);
    fclose(temp_file);

    file_line_node* file_lines = read_file_lines("dump_dictionary_test.txt");
    remove("dump_dictionary_test.txt");

    EXPECT_NOT_NULL(file_lines);
    int count = 0;
    auto current_file_line = file_lines;
    do
    {
        EXPECT_NE(count++, file_lines->line_number);
        EXPECT_NOT_NULL(file_lines->line_content);
        file_lines = file_lines->next;
    } while (file_lines != nullptr);

    free_file_lines(file_lines);
    
    dictionary_unit_test_method_cleanup();
}

