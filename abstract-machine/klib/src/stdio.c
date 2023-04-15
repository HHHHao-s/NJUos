#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char buf[1024];
  vsprintf(buf, fmt, ap);
  va_end(ap);
  putstr(buf);
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap)
{
  
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
      {
        itoa(pout, va_arg(ap, int));
        for (; (*pout) != '\0'; (pout++));
      
        break;
      }

        
      case 's':
      {
        strcpy(pout, va_arg(ap, char *));
        for (; (*pout) != '\0'; (pout++));
        break;
      }
        
      case 'p':
      {
        *pout++ = '0';
        *pout++ = 'x';
        unsigned long i = va_arg(ap, unsigned long);
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
    
  }
  *pout = '\0';
  return 0;
}

int sprintf(char *out, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsprintf(out, fmt, ap);
  va_end(ap);
  return 0;
}

int snprintf(char *out, size_t n, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(out,n,  fmt, ap);
  va_end(ap);
  return 0;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap)
{
  char buf[1024];
  char *pout = buf;

  for (const char *p = fmt; *p &&  pout-buf<n;)
  {
    char c = *p++;
    switch (c)
    {
    case '%':
      /* code */
      switch (*p++)
      {
      case 'd':{
        itoa(pout, va_arg(ap, int));
        for (; (*pout) != '\0'; (pout++));
        break;}
        
      case 's':{
        strcpy(pout, va_arg(ap, char *));
        for (; (*pout) != '\0'; (pout++));
        break;
      }
      case 'p':
      {
        *pout++ = '0';
        *pout++ = 'x';
        unsigned long i = va_arg(ap, unsigned long);
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
    
  }
  
  
  strncpy(out,buf, n);
  return 0;
}

#endif
