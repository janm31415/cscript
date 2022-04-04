#pragma once

#if defined(cscript_EXPORTS)
#  define COMPILER_CSCRIPT_API __declspec(dllexport)
#else
#  define COMPILER_CSCRIPT_API __declspec(dllimport)
#endif