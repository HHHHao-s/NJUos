#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>

void splash();
AM_INPUT_KEYBRD_T print_key();
static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}
void flash(int, int);