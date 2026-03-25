#ifndef __STRING_H__
#define __STRING_H__

#include "stdint.h"

const char* strchr(const char* str, char c);
const char* strrchr(const char* str, char c);
char* strcpy(char* dst, const char* src);
unsigned int strlen(const char* str);

void far* memcpy(void far* dst, const void far* src, uint16_t count);
void far* memset(void far* ptr, int value, uint16_t count);
int memcmp(const void far* ptr1, const void far* ptr2, uint16_t count);

#endif
