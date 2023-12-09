// ReSharper disable CppIncompatiblePointerConversion
// ReSharper disable CppClangTidyClangDiagnosticIncompatibleFunctionPointerTypes
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "error.h"
#include "memory.h"
#include "stacks.h"

const int clump_size = 25; 

/**
 * \brief Return an item in a stack
 * \param stack_instance pointer to stack
 * \param index index of item to get
 * \return data at index of stack
 */
void* item_at(const instance_data_ptr stack_instance, const long index)
{
    assert(stack_instance != NULL);
    assert(stack_instance->item_array != NULL);
    assert(index >= 0 && index < stack_instance->stack_size);
    assert(stack_instance->item_size >= 0);

    uint8_t* item_ptr = static_cast<uint8_t*>(stack_instance->item_array);
    item_ptr += static_cast<size_t>(index) * stack_instance->item_size;
    return item_ptr;
}

/**
 * \brief Push an item onto a stack
 * \param stack_instance pointer to stack instance
 * \param value value to place on stack
 * \return number of items in stack
 */
long push(const instance_data_ptr stack_instance, const void* value)
{
    assert(stack_instance != NULL);
    assert(stack_instance->item_size > 0);
    assert(stack_instance->index + 1 >= 0);
    assert(value != NULL);

    if (stack_instance->index + 1 >= stack_instance->stack_size)
    {
        if (stack_instance->stack_size == 0)
        {
            stack_instance->stack_size += clump_size;
            void* tmp = MALLOC((static_cast<size_t>(stack_instance->stack_size) * stack_instance->item_size));
            if (tmp == NULL)
            {
                error2(error_out_of_memory, "Can't expand stack.");
                exit(-1);  // NOLINT(concurrency-mt-unsafe)
            }
            stack_instance->item_array = tmp;
        }
        else
        {
            stack_instance->stack_size += clump_size;
            void* tmp = REALLOC(stack_instance->item_array, (static_cast<size_t>(stack_instance->stack_size) * stack_instance->item_size));
            if (tmp == NULL)
            {
                error2(error_out_of_memory, "Can't expand stack.");
                exit(-1);  // NOLINT(concurrency-mt-unsafe)
            }
            stack_instance->item_array = tmp;
        }
#ifdef _DEBUG
        uint8_t* item_ptr = static_cast<uint8_t*>(stack_instance->item_array);
        item_ptr += static_cast<size_t>(stack_instance->index + 1) * stack_instance->item_size;
        memset(item_ptr, 0xCC, static_cast<size_t>(stack_instance->stack_size - (stack_instance->index + 1)) * stack_instance->item_size);
#endif

    }
    unsigned char* item_ptr = static_cast<unsigned char*>(stack_instance->item_array);
    // item_ptr += (size_t)(stack_instance->index + 1) * stack_instance->item_size;
    item_ptr += (static_cast<size_t>(stack_instance->index) + 1) * stack_instance->item_size;
    memcpy(item_ptr, value, stack_instance->item_size);
    return ++(stack_instance->index);
}

/**
 * \brief Pop an item from the stack
 * \param stack_instance pointer to stack instance
 * \return address of item that is popped from the stack 
 */
void* pop(const instance_data_ptr stack_instance)
{
    assert(stack_instance != NULL);
    assert(stack_instance->item_array != NULL);
    assert(stack_instance->index >= 0 && stack_instance->index < stack_instance->stack_size);
    assert(stack_instance->item_size >= 0);

    if (stack_instance->index >= 0)
    {
        void* item_ptr = item_at(stack_instance, stack_instance->index);
        stack_instance->index--;
        return item_ptr;
    }
    return NULL;
}

/**
 * \brief return Retrieve item at the top of a stack
 * \param stack_instance pointer to stack instance
 * \return top element of stack
 */
void* top(const instance_data_ptr stack_instance)
{
    assert(stack_instance != NULL);
    assert(stack_instance->item_size >= 0);

    return item_at(stack_instance, stack_instance->index);
}

/**
 * \brief Clear the contents of a stack
 * \param stack_instance pointer to stack instance
 */
void clear_stack(const instance_data_ptr stack_instance)
{
    assert(stack_instance != NULL);
    while (stack_instance->index >= 0)
    {
        pop(stack_instance);
    }
}

/**
 * \brief Create a stack
 * \param item_size size of 1 element
 * \return newly created stack
 */
stack_ptr create_stack(const size_t item_size)
{
    const instance_data_ptr data = static_cast<instance_data_ptr>(MALLOC(sizeof(instance_data)));
    if (data == NULL)
    {
        error2(error_out_of_memory, "Can't create stack.");
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }
    memset(data, 0, sizeof(instance_data));
    data->index = -1;
    data->stack_size = 0;
    data->item_size = static_cast<long>(item_size);
    data->item_array = NULL;

    const stack_ptr sp = static_cast<stack_ptr>(MALLOC(sizeof(stack)));
    if (sp == NULL)
    {
        error2(error_out_of_memory, "Can't create stack.");
        exit(-1);  // NOLINT(concurrency-mt-unsafe)
    }
    memset(sp, 0, sizeof(stack));

    sp->instance = data;
    sp->push = push;
    sp->pop = pop;
    sp->top = top;
    sp->clear = clear_stack;
    sp->item_at = item_at;

    return sp;
}

/**
 * \brief FREE an allocated stack
 * \param sp stack to FREE 
 */
void free_stack(const stack_ptr sp)
{
    assert(sp != NULL);
    assert(sp->instance != NULL);

    if (sp->instance->item_array != NULL)
    {
#ifdef _DEBUG
        memset(sp->instance->item_array, 0xCC, static_cast<size_t>(sp->instance->item_size) * sp->instance->stack_size);
#endif
        FREE(sp->instance->item_array);
        sp->instance->item_array = NULL;
    }
#ifdef _DEBUG
    memset(sp->instance, 0xCC, sizeof(instance_data));
#endif
    FREE(sp->instance);
    sp->instance = NULL;

#ifdef _DEBUG
    memset(sp, 0xCC, sizeof(stack));
#endif
    FREE(sp);
}
