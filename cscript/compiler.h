#pragma once

#include "namespace.h"
#include "vm/vmcode.h"

#include "cscript_api.h"
#include "parse.h"

COMPILER_BEGIN

COMPILER_CSCRIPT_API void compile(VM::vmcode& code, const Program& prog);

COMPILER_END