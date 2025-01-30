#include <ctype.h>
#ifndef _WIN32
#include <stddef.h>
#endif

/// <summary>
/// Hash_function
/// </summary>
/// <param name="key">The string.</param>
/// <returns>unsigned long long.</returns>
unsigned int hash_function(const char* key)
{
    unsigned int hash = 0;

    for (unsigned long index = 0; key[index]; ++index) {
        hash += index % 2 ? tolower(key[index]) : toupper(key[index]);
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

unsigned int hash_pointer_function(const void* key)
{
    unsigned int hash = 0;

    for (size_t index = 0; index < sizeof(void*); ++index) {
        auto shift = 8 * index;
        const unsigned char b = (((unsigned long long)key) & (0xFFULL << shift)) >> shift;  // NOLINT(clang-diagnostic-implicit-int-conversion)

        hash += b;
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}
