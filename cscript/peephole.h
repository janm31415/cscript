#pragma once

#include "namespace.h"

#include "cscript_api.h"

#include "vm/vmcode.h"

COMPILER_BEGIN

COMPILER_CSCRIPT_API void peephole_optimization(VM::vmcode& code);

COMPILER_END