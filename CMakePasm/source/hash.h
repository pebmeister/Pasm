#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

extern unsigned int hash_function(const char* key);
extern unsigned int hash_pointer_function(const void* key);

#ifdef __cplusplus
}
#endif
