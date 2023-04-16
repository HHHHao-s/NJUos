#include <game.h>

#define SIDE 16
static int w, h, ori_x, ori_y;

static void init()
{
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;

  ori_x = w / SIDE / 2;
  ori_y = h / SIDE / 2;
}

static void draw_tile(int x, int y, int w, int h, uint32_t color)
{
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
      .x = x,
      .y = y,
      .w = w,
      .h = h,
      .sync = 1,
      .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++)
  {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}

void splash()
{
  init();

  for (int x = 0; x * SIDE <= w; x++)
  {
    for (int y = 0; y * SIDE <= h; y++)
    {
      if ((x & 1) ^ (y & 1))
      {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0x0000ff); // blue
      }
      else
      {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
    }
  }
  flash(0, 0);
}

void flash(int off_x, int off_y)
{
  int x = ori_x;
  int y = ori_y;
  if ((x & 1) ^ (y & 1)) // draw the past block
  {
    draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0x0000ff); // blue
  }
  else
  {
    draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
  }

  // draw the new block
  if (ori_x + off_x < w / SIDE && ori_x + off_x > 0)
  { // test if beyond the bound
    ori_x += off_x;
  }
  if (ori_y + off_y < h / SIDE && ori_y + off_y > 0)
  { // test if beyond the bound
    ori_y += off_y;
  }
  draw_tile(ori_x * SIDE, ori_y * SIDE, SIDE, SIDE, 0x00ff00); // green
}
