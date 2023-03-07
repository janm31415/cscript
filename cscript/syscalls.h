#ifndef CSCRIPT_SYSCALLS_H
#define CSCRIPT_SYSCALLS_H

#include "cscript.h"
#include "limits.h"

int cscript_write(int fd, const void* buffer, unsigned int count);

int cscript_read(int fd, void* buffer, unsigned int buffer_size);

int cscript_open_output_file(const char* filename);

int cscript_open_input_file(const char* filename);

int cscript_close_file(int fd);

long cscript_tell(int fd);

const char* cscript_getenv(const char* name);

int cscript_putenv(const char* name, const char* value);

int cscript_file_exists(const char* filename);

void cscript_fixnum_to_char(char* buffer, cscript_fixnum fx);
void cscript_fixnum_to_hex_char(char* buffer, cscript_fixnum fx);
void cscript_fixnum_to_binary_char(char* buffer, cscript_fixnum fx);
void cscript_fixnum_to_oct_char(char* buffer, cscript_fixnum fx);
void cscript_flonum_to_char(char* buffer, cscript_flonum fl);
void cscript_flonum_to_char_scientific(char* buffer, cscript_flonum fl);
void cscript_memsize_to_char(char* buffer, cscript_memsize s);
void cscript_int_to_char(char* buffer, int i);

#endif //CSCRIPT_SYSCALLS_H