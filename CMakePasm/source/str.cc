// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "memory.h"
#include "str.h"

/**
 * \brief Normalize a string that may contain escape chars
 * \param str string to normalize
 * \return a newly allocated string
 */
char* sanitize_string(char* str)
{
    const int len = (int)strlen(str) + 1;

    char* out_str = (char*)MALLOC(len);
    char* tmp_str = out_str;
    unsigned char esc_char = 0;

    if (out_str == NULL)
    {
        error2(error_out_of_memory, "Can't sanitize string");
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }
    memset(out_str, 0, len);

    if (str == NULL)
    {
        error(error_source_string_null);
        return NULL;
    }

    while (*str)
    {
        if (*str == '\\')
        {
            switch (tolower(*(++str)))
            {
            case 'a':
                esc_char = '\a';
                break;

            case 'b':
                esc_char = '\b';
                break;

            case 'f':
                esc_char = '\f';
                break;

            case 'v':
                esc_char = '\v';
                break;

            case 'r':
                esc_char = '\r';
                break;

            case 'n':
                esc_char = '\n';
                break;

            case 't':
                esc_char = '\t';
                break;

            case 'x':
                str++;
                if (*str == 0)
                {
                    error(error_unrecognized_escape_sequence);
                    break;
                }
                while (isxdigit(*str) && isxdigit(*(str + 1)))
                {
                    char temp[3] = { 0 };
                    temp[0] = *str;
                    temp[1] = *(str + 1);
                    *tmp_str++ = (char)(int)strtol(temp, NULL, 16);
                    str += 2;
                }
                continue;

            case '\'':
            case '\"':
            case '\\':
                esc_char = *str & 0xFF;
                break;

            default:
                esc_char = '?';
                error(error_unrecognized_escape_sequence);
                break;
            }
            *tmp_str++ = (char)esc_char;
            str++;
        }
        else if (*str)
        {
            *tmp_str++ = *str++;
        }
    }

    return out_str;
}
