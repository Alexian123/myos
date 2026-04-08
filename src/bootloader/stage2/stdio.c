#include "stdio.h"
#include "x86.h"

#include <stdarg.h>
#include <stdbool.h>

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
    PRINTF_LENGTH_SHORT_SHORT,
    PRINTF_LENGTH_SHORT,
    PRINTF_LENGTH_LONG,
    PRINTF_LENGTH_LONG_LONG
};

static const unsigned SCREEN_WIDTH = 80;
static const unsigned SCREEN_HEIGHT = 25;
static const uint8_t DEFAULT_COLOR = 0x7;
static const char HEX_CHARS[] = "0123456789abcdef";

static uint8_t* g_screenBuffer = (uint8_t*)0xB8000;
static int g_screenX = 0;
static int g_screenY = 0;

static void _putchr(int x, int y, char c);
static void _putcolor(int x, int y, uint8_t color);
static char _getchr(int x, int y);
static uint8_t _getcolor(int x, int y);
static void _setcursor(int x, int y);
static void _scrollback(int lines);
static void _printf_unsigned(unsigned long long number, int radix);
static void _printf_signed(long long number, int radix);

void clrscr() {
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            _putchr(x, y, '\0');
            _putcolor(x, y, DEFAULT_COLOR);
        }

    g_screenX = 0;
    g_screenY = 0;
    _setcursor(g_screenX, g_screenY);
}

void putc(char c) {
    switch (c)
    {
        case '\n':
            g_screenX = 0;
            ++g_screenY;
            break;
    
        case '\t':
            for (int i = 0; i < 4 - (g_screenX % 4); ++i) {
                putc(' ');
            }
            break;

        case '\r':
            g_screenX = 0;
            break;

        default:
            _putchr(g_screenX, g_screenY, c);
            ++g_screenX;
            break;
    }

    if (g_screenX >= SCREEN_WIDTH) {
        ++g_screenY;
        g_screenX = 0;
    }
    if (g_screenY >= SCREEN_HEIGHT) {
        _scrollback(1);
    }

    _setcursor(g_screenX, g_screenY);
}

void puts(const char* str) {
    while (*str) {
        putc(*str);
        ++str;
    }
}

void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    enum PrintfState state = PRINTF_STATE_NORMAL;
    enum PrintfLength length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;
    bool number = false;

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
                putc((char)va_arg(args, int));
                break;
            
            case 's':
                puts(va_arg(args, const char*));
                break;

            case '%':
                putc('%');
                break;

            case 'd':
            case 'i':
                radix = 10;
                sign = true;
                number = true;
                break;

            case 'u':
                radix = 10;
                sign = false;
                number = true;
                break;

            case 'x':
            case 'X':
            case 'p':
                radix = 16;
                sign = false;
                number = true;
                break;

            case 'o':
                radix = 8;
                sign = false;
                number = true;
                break;

            case 'b':
                radix = 2;
                sign = false;
                number = true;

            default:    // ignore invalid specifiers
                break;
            }

            if (number) {
                if (sign) {
                    switch (length)
                    {
                    case PRINTF_LENGTH_SHORT_SHORT:
                    case PRINTF_LENGTH_SHORT:
                    case PRINTF_LENGTH_DEFAULT:     
                        _printf_signed(va_arg(args, int), radix);
                        break;

                    case PRINTF_LENGTH_LONG:        
                        _printf_signed(va_arg(args, long), radix);
                        break;

                    case PRINTF_LENGTH_LONG_LONG:   
                        _printf_signed(va_arg(args, long long), radix);
                        break;
                    default:
                        break;
                    }
                } else {
                    switch (length)
                    {
                    case PRINTF_LENGTH_SHORT_SHORT:
                    case PRINTF_LENGTH_SHORT:
                    case PRINTF_LENGTH_DEFAULT:
                        _printf_unsigned(va_arg(args, unsigned int), radix);
                        break;
                                                    
                    case PRINTF_LENGTH_LONG:        
                        _printf_unsigned(va_arg(args, unsigned  long), radix);
                        break;

                    case PRINTF_LENGTH_LONG_LONG:   
                        _printf_unsigned(va_arg(args, unsigned  long long), radix);
                        break;
                    default:
                        break;
                    }
                }
            }

            // reset state
            state = PRINTF_STATE_NORMAL;
            length = PRINTF_LENGTH_DEFAULT;
            radix = 10;
            sign = false;
            number = false;
            break;

        default:
            break;
        }

        ++fmt;
    }
    va_end(args);
}

void print_buffer(const char* msg, const void* buffer, uint32_t count) {
    const uint8_t* u8Buffer = (const uint8_t*)buffer;
    puts(msg);
    for (uint16_t i = 0; i < count; i++) {
        putc(HEX_CHARS[u8Buffer[i] >> 4]);
        putc(HEX_CHARS[u8Buffer[i] & 0xF]);
    }
    puts("\n");
}

void _putchr(int x, int y, char c) {
    g_screenBuffer[2 * (y * SCREEN_WIDTH + x)] = c;
}

void _putcolor(int x, int y, uint8_t color) {
    g_screenBuffer[2 * (y * SCREEN_WIDTH + x) + 1] = color;
}

char _getchr(int x, int y) {
    return g_screenBuffer[2 * (y * SCREEN_WIDTH + x)];
}

uint8_t _getcolor(int x, int y) {
    return g_screenBuffer[2 * (y * SCREEN_WIDTH + x) + 1];
}

void _setcursor(int x, int y) {
    int pos = y * SCREEN_WIDTH + x;
    x86_outb(0x3D4, 0x0F);
    x86_outb(0x3D5, (uint8_t)(pos & 0xFF));
    x86_outb(0x3D4, 0x0E);
    x86_outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void _scrollback(int lines) {
    for (int y = lines; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            _putchr(x, y - lines, _getchr(x, y));
            _putcolor(x, y - lines, _getcolor(x, y));
        }
    }
    for (int y = SCREEN_HEIGHT - lines; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            _putchr(x, y, '\0');
            _putcolor(x, y, DEFAULT_COLOR);
        }
    }
    g_screenY -= lines;
}

void _printf_unsigned(unsigned long long number, int radix) {
    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = HEX_CHARS[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        putc(buffer[pos]);
}

void _printf_signed(long long number, int radix) {
    if (number < 0) {
        putc('-');
        _printf_unsigned(-number, radix);
    }
    else _printf_unsigned(number, radix);
}
