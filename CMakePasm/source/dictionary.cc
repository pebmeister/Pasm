// ***********************************************************************
// Author           : Paul Baxter
// ***********************************************************************

// ReSharper disable CppClangTidyClangDiagnosticExtraSemiStmt
// ReSharper disable CppClangTidyConcurrencyMtUnsafe
// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyClangDiagnosticIntToVoidPointerCast
// ReSharper disable CppClangTidyPerformanceNoIntToPtr
// ReSharper disable CppClangTidyClangDiagnosticBadFunctionCast
// ReSharper disable CppClangTidyClangDiagnosticVoidPointerToIntCast
// ReSharper disable CppPointerConversionDropsQualifiers
// ReSharper disable CppClangTidyCertErr33C
// ReSharper disable CppClangTidyClangDiagnosticIncompatiblePointerTypesDiscardsQualifiers

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dictionary.h"
#include "error.h"
#include "hash.h"
#include "memory.h"

#pragma warning(disable: 4311 4090 4996 )

enum
{
    initial_size = 1 * 1024,
    growth_factor = 2
};

// How full can the dictionary get before we grow it
#define MAX_LOAD_FACTOR (.90)

/// <summary>
/// dictionary initialization code used in both DictCreate and grow.
/// </summary>
/// <param name="size">The initial number of elements capacity.</param>
/// <param name="element_size">Size of the element.</param>
/// <param name="key_type">type of key 0 = string</param>
/// <returns>Dictionary.</returns>
dictionary_ptr internal_dict_create(const int size, const int element_size, const int key_type)
{
    const dictionary_ptr dictionary = static_cast<dictionary_ptr>(MALLOC(sizeof (dict)));

    if (dictionary == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }

    memset(dictionary, 0, sizeof (dict));

    dictionary->size = size;
    dictionary->element_size = element_size;
    dictionary->key_type = key_type;
    dictionary->table = static_cast<element_ptr*>(MALLOC(sizeof(element_ptr) * dictionary->size));
    if (dictionary->table == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    memset(dictionary->table, 0, sizeof(element_ptr) * dictionary->size);

    return dictionary;
}

/// <summary>
/// Dictionary creation.
/// </summary>
/// <returns>created dictionary.</returns>
dictionary_ptr dict_create(const int element_size, const int key_type)
{
    return internal_dict_create(initial_size, element_size, key_type);
}

/// <summary>
/// Dictionary destroy.
/// </summary>
/// <param name="d">The dictionary.</param>
void dict_destroy(const dictionary_ptr d)
{
    if (d == NULL) return;

    for (int index = 0; index < d->size; index++)
    {
        element_ptr next;
        for (element_ptr element = d->table[index]; element != 0; element = next)
        {
            next = element->next;
            if (d->key_type == 0)
            {
                FREE(element->key);  // NOLINT(clang-diagnostic-cast-qual)
            }
            FREE(element->value);
            FREE(element);
        }
    }
    if (d->table != NULL)
    {
        FREE(d->table);
    }
    FREE(d);
}

/// <summary>
/// Grows the specified dictionary returns new dictionary.
/// </summary>
/// <param name="d">The dictionary.</param>
static dictionary_ptr grow(const dictionary_ptr d)
{
    // ReSharper disable once CppTooWideScope
    dictionary_ptr d2 = internal_dict_create(d->size * growth_factor, d->element_size, d->key_type);
    if (d2 == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    d2->collisions = 0;
    for (int index = 0; index < d->size; index++)
    {
        for (element_ptr element = d->table[index]; element != 0; element = element->next)
        {
            /* note: this recopies everything */
            /* a more efficient implementation would
             * to avoid this problem */
            dict_insert(&d2, element->key, element->value);
        }
    }

    free(d->table);
    free(d);

    return d2;
}

/// <summary>
/// insert a new key-value pair into an existing dictionary.
/// dictionary may grow
/// </summary>
/// <param name="dd">The address of dictionary pointer.</param>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
void* dict_insert(dictionary_ptr* dd, void* key, void* value)
{
    // ReSharper disable once CppLocalVariableMayBeConst
    dictionary_ptr d = *dd;

    // ReSharper disable once CppLocalVariableMayBeConst
    element_ptr e =  static_cast<element_ptr>(MALLOC(sizeof(element)));
    if (e == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    // IMPORTANT: This sets the next to NULL
    memset(e, 0, sizeof(element));

    if (d->key_type == 0)
    {
        e->key = (char*) STRDUP(static_cast<char*>(key));
        if (e->key == NULL)
        {
            error(error_out_of_memory);
            exit(-1);
        }
    }
    else
    {
        // ReSharper disable once CppPointerConversionDropsQualifiers
        e->key = key;  // NOLINT(clang-diagnostic-incompatible-pointer-types-discards-qualifiers)
    }
    e->value = MALLOC(d->element_size);
    if (e->value == NULL)
    {
        error(error_out_of_memory);
        exit(-1);
    }
    memcpy(e->value, value, d->element_size);
    if (d->key_type == 0)
    {
        const unsigned long h = hash_function(static_cast<const char*>(e->key)) % d->size;
        e->next = d->table[h];
        if (e->next)
            d->collisions++;

        d->table[h] = e;
        d->number_elements++;
    }
    else
    {
        const unsigned long h = hash_pointer_function(e->key) % d->size;
        e->next = d->table[h];
        if (e->next)
            d->collisions++;

        d->table[h] = e;
        d->number_elements++;
    }
    /* grow table if there is not enough room */
    if (d->number_elements >= static_cast<long>((double)d->size * MAX_LOAD_FACTOR))
    {
        *dd = grow(d);
        return dict_search(*dd, key);
    }

    return e->value;
}

/// <summary>
/// return the most recently inserted value associated with a key.
/// or NULL if no matching key is present
/// </summary>
/// <param name="d">The d.</param>
/// <param name="key">The key.</param>
/// <returns>void *.</returns>
void* dict_search(const dictionary_ptr d, const void* key)
{
    if (d->key_type == 0)
    {
        const unsigned long h = hash_function(static_cast<const char*>(key)) % d->size;
        for (element_ptr e = d->table[h]; e != 0; e = e->next)
        {
            if (!stricmp(static_cast<char*>(e->key), (char*)key))
            {
                /* got it */
                return e->value;
            }
        } 
    }
    else
    {
        const unsigned long h = hash_pointer_function(key) % d->size;
        for (element_ptr e = d->table[h]; e != 0; e = e->next)
        {
            if (e->key == key)
            {
                /* got it */
                return e->value;
            }
        }
    }
    return NULL;
}

/// <summary>
/// delete the most recently inserted record with the given key
/// if there is no such record, has no effect
/// </summary>
/// <param name="d">The dictionary.</param>
/// <param name="key">The key.</param>
void dict_delete(const dictionary_ptr d, const void* key)
{
    if (d->key_type == 0)
    {
        const unsigned long h = hash_function((char*)key) % d->size;

        for (element_ptr* prev = &d->table[h]; *prev != 0; prev = &(*prev)->next)
        {
            if (!stricmp(static_cast<char*>((*prev)->key), (char*)key))
            {
                const element_ptr e = *prev;
                *prev = e->next;

                // ReSharper disable once CppPointerConversionDropsQualifiers
                char* p = static_cast<char*>(e->key);  // NOLINT(clang-diagnostic-cast-qual)
                // ReSharper disable once CppPointerConversionDropsQualifiers
                FREE(p);  // NOLINT(clang-diagnostic-incompatible-pointer-types-discards-qualifiers)
                FREE(e->value);
                (d->number_elements)--;
                return;
            }
        }
    }
    else
    {
        const unsigned long h = hash_pointer_function(key) % d->size;

        for (element_ptr* prev = &d->table[h]; *prev != 0; prev = &(*prev)->next)
        {
            if ((*prev)->key == key)
            {
                const element_ptr e = *prev;
                *prev = e->next;
                FREE(e);
                d->number_elements--;
                return;
            }
        }
    }
}

/**
 * \brief Dump contents of a dictionary
 * \param d dictionary to dump for
 * \param p element printer
 * \param file file to print to
 */
void dump_dictionary(const dictionary_ptr d, const print_element p, FILE* file)
{
    if (d == NULL) return;
    if (file == NULL) return;

    long count = 0;
    for (int index = 0; index < d->size; index++)
    {
        element_ptr next;
        for (element_ptr element = d->table[index]; element != 0; element = next)
        {
            count++;

            if (p)
            {
                p(element, file);
            }
            else
            {
                fprintf(file, "[%d]\n", index);

                if (d->key_type == 0)
                {
                    fprintf(file, "    Key   %s\n", static_cast<char*>(element->key));
                }
                else
                {
                    fprintf(file, "    Key   %8p\n", element->key);
                }
                fprintf(file, "    Value %8p\n", element->value);
                fprintf(file, "    Next %8p\n\n", static_cast<void*>(element->next));
                fprintf(file, "\n");
            }
            next = element->next;
        }
    }
    assert(count == d->number_elements);
}
