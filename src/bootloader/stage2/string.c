#include "string.h"
#include "stdint.h"

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

void far* memcpy(void far* dst, const void far* src, uint16_t count) {
    uint8_t far* u8Dst = (uint8_t far*)dst;
    const uint8_t far* u8Src = (const uint8_t far*)src;
    
    if (dst == NULL || src == NULL || count == 0) {
        return 0;
    }

    for (uint16_t i = 0; i < count; ++i) {
        u8Dst[i] = u8Src[i];
    }
    return dst;
}

void far* memset(void far* ptr, int value, uint16_t count) {
    uint8_t far* u8Ptr = (uint8_t far*)ptr;

    for (uint16_t i = 0; i < count; ++i) {
        u8Ptr[i] = (uint8_t)value;
    }

    return ptr;
}

int memcmp(const void far* ptr1, const void far* ptr2, uint16_t count) {
    const uint8_t far* u8Ptr1 = (const uint8_t far*)ptr1;
    const uint8_t far* u8Ptr2 = (const uint8_t far*)ptr2;

    for (uint16_t i = 0; i < count; ++i) {
        if (u8Ptr1[i] != u8Ptr2[i]) {
            return 1;
        }
    }
    return 0;
}
