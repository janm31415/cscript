#include "syscalls.h"

#ifdef _WIN32
#include <io.h>
#include <sys\stat.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int cscript_write(int fd, const void* buffer, unsigned int count)
  {
  if (fd < 0)
    return 0;
#ifdef _WIN32
  return _write(fd, buffer, count);
#else
  return write(fd, buffer, count);
#endif
  }

int cscript_read(int fd, void* buffer, unsigned int buffer_size)
  {
  if (fd < 0)
    return 0;
#ifdef _WIN32
  return _read(fd, buffer, buffer_size);
#else
  return read(fd, buffer, buffer_size);
#endif
  }

int cscript_open_output_file(const char* filename)
  {
#ifdef _WIN32
  return _open(filename, _O_CREAT | O_WRONLY | O_TRUNC, _S_IREAD | _S_IWRITE);
#elif defined(__APPLE__)
  return open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IREAD | S_IWRITE);
#else // unix
  return open(filename, O_CREAT | O_WRONLY | O_TRUNC, __S_IREAD | __S_IWRITE);
#endif
  }

int cscript_open_input_file(const char* filename)
  {
#ifdef _WIN32
  return _open(filename, _O_RDONLY, _S_IREAD | _S_IWRITE);
#elif defined(__APPLE__)
  return open(filename, O_RDONLY, S_IREAD | S_IWRITE);
#else // unix
  return open(filename, O_RDONLY, __S_IREAD | __S_IWRITE);
#endif
  }

int cscript_close_file(int fd)
  {
  if (fd < 0)
    return 0;
#ifdef _WIN32
  return _close(fd);
#else
  return close(fd);
#endif
  }

long cscript_tell(int fd)
  {
#ifdef _WIN32
  return _tell(fd);
#else
  return lseek(fd, 0, SEEK_CUR);
#endif
  }

const char* cscript_getenv(const char* name)
  {
  return getenv(name);
  }

int cscript_putenv(const char* name, const char* value)
  {
#ifdef _WIN32
  return (int)_putenv_s(name, value);
#else
  return setenv(name, value, 1);
#endif
  }

int cscript_file_exists(const char* filename)
  {
  int res = 0;
  FILE* f = fopen(filename, "r");
  if (f)
    {
    res = 1;
    fclose(f);
    }
  return res;
  }

void cscript_fixnum_to_char(char* buffer, cscript_fixnum fx)
  {
  sprintf(buffer, "%lld", fx);
  }

void cscript_fixnum_to_binary_char(char* str, cscript_fixnum a)
  {
  if (a == 0)
    {
    str[0] = '0';
    str[1] = 0;
    }
  else
    {
    int len = cast(int, ceil(log2(cast(double, a))));
    char* s = str + len;
    *s-- = 0;
    while (a > 0)
      {
      *s-- = a % 2 + '0';
      a /= 2;
      }
    }
  }

void cscript_fixnum_to_hex_char(char* buffer, cscript_fixnum fx)
  {
  sprintf(buffer, "%llx", fx);
  }

void cscript_fixnum_to_oct_char(char* buffer, cscript_fixnum fx)
  {
  sprintf(buffer, "%llo", fx);
  }

void cscript_flonum_to_char(char* buffer, cscript_flonum fl)
  {
  sprintf(buffer, "%f", fl);
  }

void cscript_flonum_to_char_scientific(char* buffer, cscript_flonum fl)
  {
  sprintf(buffer, "%.17e", fl);
  }

void cscript_memsize_to_char(char* buffer, cscript_memsize s)
  {
  sprintf(buffer, "%d", s);
  }

void cscript_int_to_char(char* buffer, int i)
  {
  sprintf(buffer, "%d", i);
  }