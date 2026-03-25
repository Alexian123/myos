#include "ctype.h"

char toupper(char c) {
    return islower(c) ? (c - 'a' + 'A') : c;
}

char tolower(char c) {
    return isupper(c) ? (c - 'A' + 'a') : c;
}

bool isupper(char c) {
    return c >= 'A' && c <= 'Z';
}

bool islower(char c) {
    return c >= 'a' && c <= 'z';
}
