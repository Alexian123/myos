#ifndef __STDIO_H__
#define __STDIO_H__

extern void putc(char c);
extern void puts(const char* str);
extern void puts_far(const char far* str);
extern void _cdecl printf(const char* fmt, ...);

#endif
