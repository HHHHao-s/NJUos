#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

int itoa(char *a, int i)
{ // return strlen
    char buf[1024];
    char *p = buf;
    int count = 0;
    int n = 0;
    while (i > 0)
    {
        n = i % 10;
        *p++ = n + '0';
        i /= 10;
        count++;
    }
    for (; p-- > buf;)
    {
        *a++ = *p;
    }
    *a = '\0';
    return count;
}

int _printf(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    char buf[4096];
    char *pout = buf;
    for (const char *p = fmt; *p;)
    {
        char c = *p++;
        switch (c)
        {
        case '%':
            /* code */
            switch (*p++)
            {
            case 'd':

                itoa(pout, va_arg(va, int));
                break;
            case 's':
                strcpy(pout, va_arg(va, char *));
                break;
            case 'p':
            {
                *pout++ = '0';
                *pout++ = 'x';
                unsigned long i = va_arg(va, unsigned long);
                char buf1[64];
                char *p = buf1;
                int n = 0;
                while (i > 0)
                {
                    n = i % 16;
                    if(n<10){
                        *p++ = n + '0';
                    }else{
                        *p++ = n + 'a' -10;
                    }
                    
                    i /= 16;
                }
                for (; p-- > buf1;)
                {
                    *pout++ = *p;
                }
            }

            break;
            default:
                break;
            }
            break;

        default:
            *pout++ = c;
            break;
        }
        for (; (*pout) != '\0'; (pout++))
            ;
    }
    *pout = '\0';
    printf(buf);
    return 0;
}

int main(int args, char **argv)
{
    char s[12];
    char *p = &s;
    printf("%p\n", p);
    _printf("%p", p);

    return 0;
}