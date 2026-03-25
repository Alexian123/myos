#ifndef __STDIO_H__
#define __STDIO_H__

void putc(char c);
void puts(const char* str);
void puts_far(const char far* str);
void _cdecl printf(const char* fmt, ...);

#endif
