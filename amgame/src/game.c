#include <game.h>

// Operating system is a C program!
int main(const char *args) {
  ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");
  
  splash();
  
  puts("Press any key to see its key code...\n");
  while (1) {
    AM_INPUT_KEYBRD_T event = print_key();
    if(event.keycode != AM_KEY_NONE){
      switch (event.keycode)
      {
      case AM_KEY_W:
        /* code */
        flash(0,-1);
        break;
      case AM_KEY_S:
        /* code */
        flash(0,1);
        break;
      case AM_KEY_A:
        /* code */
        flash(-1,0);
        break;
      case AM_KEY_D:
        /* code */
        flash(1,0);
        break;  
      
      default:
        break;
      }
    }
  }
  return 0;
}
