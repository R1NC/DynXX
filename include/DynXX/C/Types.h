#ifndef DYNXX_INCLUDE_C_TYPES_H_
#define DYNXX_INCLUDE_C_TYPES_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "Macro.h"

DYNXX_EXTERN_C_BEGIN
typedef unsigned char byte;
typedef intptr_t address;
typedef void* RawPtr;

DYNXX_EXTERN_C_END

#endif // DYNXX_INCLUDE_C_TYPES_H_
