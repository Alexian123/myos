#include "string.h"

#include <stdint.h>
#include <stddef.h>

const char* strchr(const char* str, char c) {
    if (str == NULL) {
        return NULL;
    }
    while (*str) {
        if (*str == c) {
            return str;
        }
        ++str;
    }
    return NULL;
}

const char* strrchr(const char* str, char c) {
    if (str == NULL) {
        return NULL;
    }

    unsigned int len = strlen(str);
    for (int i = len - 1; i >= 0; --i) {
        if (str[i] == c) {
            return str + i;
        }
    }
    return NULL;
}

char* strcpy(char* dst, const char* src) {
    char* saveDst = dst;
    
    if (dst == NULL) {
        return NULL;
    }
    if (src == NULL) {
        *dst = '\0';
        return dst;
    }

    while (*src) {
        *dst = *src;
        ++src;
        ++dst;
    }
    *dst = '\0';
    
    return saveDst;
}

unsigned int strlen(const char* str) {
    unsigned int len = 0;
    if (str == NULL) {
        return len;
    }
    while (*str) {
        ++len;
        ++str;
    }
    return len;
}