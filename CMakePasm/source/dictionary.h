#pragma once
#include <stdio.h>

// ***********************************************************************
// Author           : Paul Baxter
//
// ***********************************************************************
/// <summary>
/// element structure
/// </summary>
#pragma once
typedef struct element
{
    /// <summary>
    /// The next element
    /// </summary>
    struct element* next;
    /// <summary>
    /// The key
    /// </summary>
    void* key;
    /// <summary>
    /// The value
    /// </summary>
    void* value;
} element, * element_ptr;

/// <summary>
/// dict struct
/// </summary>
typedef struct dict
{
    int key_type;
    /// <summary>
    /// size of the pointer table
    /// </summary>
    long size;
    /// <summary>
    /// The number elements stored
    /// </summary>
    long number_elements;
    /// <summary>
    /// The element size
    /// </summary>
    long element_size;
    /// <summary>
    /// The number of collisions
    /// </summary>
    long collisions;
    /// <summary>
    /// The table
    /// </summary>    
    element** table;

} dict, * dictionary_ptr;

/* create a new empty dictionary */
extern dictionary_ptr dict_create(int element_size, int key_type);

/* destroy a dictionary */
extern void dict_destroy(dictionary_ptr);

/*
   insert a new key-value pair into an existing dictionary
   the dictionary may expanded which why address of pointer is passed
*/
extern void* dict_insert(dictionary_ptr*, void* key, void* value);

/* return the most recently inserted value associated with a key */
/* or 0 if no matching key is present */
extern void* dict_search(dictionary_ptr, const void* key);

/* delete the most recently inserted record with the given key */
/* if there is no such record, has no effect */
extern void dict_delete(dictionary_ptr, const void* key);

typedef int (*print_element)(element_ptr e, FILE* file);

extern void dump_dictionary(dictionary_ptr d,  print_element p, FILE* file);
