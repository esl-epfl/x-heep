
#include <stdio.h>
#include <stdint.h>

//#include "n_uart.h"

//  Retargeting (for PRINTF etc)
// ------------------------------------

#ifdef __PRINTF_TAG_PTR_DEFINED

int __getchar(__printf_tag_ptr p_file)
{
    uint8_t input = 0;
    // TODO?
    return input;
}

int __putchar(int ch, __printf_tag_ptr p_file)
{
    // UNUSED_PARAMETER(p_file);
    //N_uart_putc(ch);
    return ch;
}
#else
int __getchar(FILE * p_file)
{
    uint8_t input = 0;
    // TODO?
    return input;
}

int __putchar(int ch, FILE * p_file)
{
    // UNUSED_PARAMETER(p_file);
    //N_uart_putc(ch);
    return ch;
}
#endif

/* X-HEEP COMMENT
int _write(int file, const char * p_char, int len)
{
    int i;

    // UNUSED_PARAMETER(file);
    for (i = 0; i < len; i++)
    {
        //N_uart_putc(*p_char++);
    }
    return len;
}
X-HEEP COMMENT END */

/* X-HEEP COMMENT
int _read(int file, char * p_char, int len)
{
    // UNUSED_PARAMETER(file);
    // TODO?
    return 1;
} X-HEEP COMMENT END */