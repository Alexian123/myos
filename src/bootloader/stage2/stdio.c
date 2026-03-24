#include "stdio.h"
#include "x86.h"

void putc(char c) {
    x86_Video_WriteCharTTY(c, 0);
}

void puts(const char* str) {
    while (*str) {
        putc(*str);
        ++str;
    }
}

void puts_far(const char far* str)
{
    while(*str)
    {
        putc(*str);
        str++;
    }
}

enum PrintfState
{
    PRINTF_STATE_NORMAL,
    PRINTF_STATE_LENGTH,
    PRINTF_STATE_LENGTH_SHORT,
    PRINTF_STATE_LENGTH_LONG,
    PRINTF_STATE_SPEC
};

enum PrintfLength
{
    PRINTF_LENGTH_DEFAULT,
    PRINTF_LENGTH_SHORT,
    PRINTF_LENGTH_SHORT_SHORT,
    PRINTF_LENGTH_LONG,
    PRINTF_LENGTH_LONG_LONG
};

static int* printf_int(int* argp, enum PrintfLength length, bool sign, int radix);

void _cdecl printf(const char* fmt, ...) {
    int *argp = (int*)&fmt;
    enum PrintfState state = PRINTF_STATE_NORMAL;
    enum PrintfLength length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;

    argp += (sizeof(fmt) / sizeof(int));
    while (*fmt) {
        switch (state)
        {
        case PRINTF_STATE_NORMAL:
            switch (*fmt)
            {
            case '%':
                state = PRINTF_STATE_LENGTH;
                break;
            
            default:
                putc(*fmt);
                break;
            }
            break;
        
        case PRINTF_STATE_LENGTH:
            switch (*fmt)
            {
            case 'h':
                length = PRINTF_LENGTH_SHORT;
                state = PRINTF_STATE_LENGTH_SHORT;
                break;

            case 'l':
                length = PRINTF_LENGTH_LONG;
                state = PRINTF_STATE_LENGTH_LONG;
                break;
            
            default:
                goto PRINTF_STATE_SPEC_;
                break;  // unreachable
            }
            break;

        case PRINTF_STATE_LENGTH_SHORT:
            if (*fmt == 'h') {
                length = PRINTF_LENGTH_SHORT_SHORT;
                state = PRINTF_STATE_SPEC;
            } else {
                goto PRINTF_STATE_SPEC_;
            }
            break;

        case PRINTF_STATE_LENGTH_LONG:
            if (*fmt == 'l') {
                length = PRINTF_LENGTH_LONG_LONG;
                state = PRINTF_STATE_SPEC;
            } else {
                goto PRINTF_STATE_SPEC_;
            }
            break;

        case PRINTF_STATE_SPEC:
PRINTF_STATE_SPEC_:
            switch (*fmt)
            {
            case 'c':
                putc((char)*argp);
                ++argp;
                break;
            
            case 's':
                if (length == PRINTF_LENGTH_LONG || length == PRINTF_LENGTH_LONG_LONG) {
                    puts_far(*(const char far**)argp);
                    argp += 2;
                } else {
                    puts(*(const char**)argp);
                    ++argp;
                }
                break;

            case '%':
                putc('%');
                break;

            case 'd':
            case 'i':
                radix = 10;
                sign = true;
                argp = printf_int(argp, length, sign, radix);
                break;

            case 'u':
                radix = 10;
                sign = false;
                argp = printf_int(argp, length, sign, radix);
                break;

            case 'x':
            case 'X':
            case 'p':
                radix = 16;
                sign = false;
                argp = printf_int(argp, length, sign, radix);
                break;

            case 'o':
                radix = 8;
                sign = false;
                argp = printf_int(argp, length, sign, radix);
                break;

            default:    // ignore invalid specifiers
                break;
            }

            // reset state
            state = PRINTF_STATE_NORMAL;
            length = PRINTF_LENGTH_DEFAULT;
            radix = 10;
            sign = false;
            break;

        default:
            break;
        }

        ++fmt;
    }
}

static const char g_hexChars[] = "0123456789abcdef";

int* printf_int(int* argp, enum PrintfLength length, bool sign, int radix) {
    char buffer[32];
    unsigned long long number;
    int number_sign = 1;
    int pos = 0;

    switch (length) {
        case PRINTF_LENGTH_SHORT_SHORT:
        case PRINTF_LENGTH_SHORT:
        case PRINTF_LENGTH_DEFAULT:
            if (sign) {
                int n = *argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long long)n;
            } else {
                number = *(unsigned int*)argp;
            }
            ++argp;
            break;

        case PRINTF_LENGTH_LONG:
            if (sign) {
                long int n = *(long int*)argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long long)n;
            } else {
                number = *(unsigned long int*)argp;
            }
            argp += 2;
            break;
        
        case PRINTF_LENGTH_LONG_LONG:
            if (sign) {
                long long int n = *(long long int*)argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (unsigned long long)n;
            } else {
                number = *(unsigned long long*)argp;
            }
            argp += 4;
            break;  

        default:
            break;
    }

    // itoa
    do {
        uint32_t rem;
        x86_Div64_32(number, radix, &number, &rem);
        buffer[pos++] = g_hexChars[rem];
    } while (number > 0);

    // add sign
    if (sign && number_sign < 0) {
        buffer[pos++] = '-';
    }
    
    // print buffer in reverse
    while (--pos >= 0) {
        putc(buffer[pos]);
    }

    return argp;
}
