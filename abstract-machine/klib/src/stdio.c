#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...)
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
          if (n < 10)
          {
            *p++ = n + '0';
          }
          else
          {
            *p++ = n + 'a' - 10;
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
  putstr(buf);
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap)
{
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  // char buf[1024];
  char *pout = out;
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
          if (n < 10)
          {
            *p++ = n + '0';
          }
          else
          {
            *p++ = n + 'a' - 10;
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
  return 0;
}

int snprintf(char *out, size_t n, const char *fmt, ...)
{
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap)
{
  panic("Not implemented");
}

#endif
