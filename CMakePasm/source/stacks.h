#pragma once

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct instance_data
{
    long index;
    long stack_size;
    long item_size;
    void* item_array;
} instance_data, * instance_data_ptr;
 
typedef struct stack
{
    instance_data_ptr instance;
    long (*push)(instance_data_ptr instance, const void* value);
    void (*clear)(instance_data_ptr instance);
    void* (*pop)(instance_data_ptr instance);
    void* (*top)(instance_data_ptr instance);
    void* (*item_at)(instance_data_ptr instance, long int index);
} stack, * stack_ptr;

extern stack_ptr create_stack(size_t item_size);
extern void free_stack(stack_ptr sp);

#ifdef __cplusplus
}
#endif
