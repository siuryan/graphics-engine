/*====================== display.c ========================
Contains functions for basic manipulation of a screen 
represented as a 2 dimensional array of colors.

A color is an ordered triple of ints, with each value standing
for red, green and blue respectively
==================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "ml6.h"
#include "display.h"


/*======== void plot() ==========
Inputs:   screen s
         color c
         int x
         int y
Returns:
Sets the color at pixel x, y to the color represented by c
Note that s[0][0] will be the upper left hand corner
of the screen.
If you wish to change this behavior, you can change the indicies
of s that get set. For example, using s[x][YRES-1-y] will have
pixel 0, 0 located at the lower left corner of the screen
====================*/
void plot(screen s, zbuffer zb, color c, int x, int y, double z) {
  int newy = YRES - 1 - y;
  z = (int)(z * 1000) / 1000;
  if ( x >= 0 && x < XRES && newy >=0 && newy < YRES &&
       zb[x][newy] <= z ) {
    s[x][newy] = c;
    zb[x][newy] = z;
  }
}

/*======== void clear_screen() ==========
Inputs:   screen s
Returns:
Sets every color in screen s to black
====================*/
void clear_screen( screen s ) {

  int x, y;
  color c;

  /* c.red = 0; */
  /* c.green = 0; */
  /* c.blue = 0; */

  c.red = 255;
  c.green = 255;
  c.blue = 255;

  for ( y=0; y < YRES; y++ )
    for ( x=0; x < XRES; x++)
      s[x][y] = c;
}

/*======== void clear_zbuffer() ==========
Inputs:   zbuffer
Returns:
Sets all entries in the zbufffer to LONG_MIN
====================*/
void clear_zbuffer( zbuffer zb ) {

  int x, y;

  for ( y=0; y < YRES; y++ )
    for ( x=0; x < XRES; x++)
      zb[x][y] = LONG_MIN;
}

/*======== void save_ppm() ==========
Inputs:   screen s
         char *file
Returns:
Saves screen s as a valid ppm file using the
settings in ml6.h
====================*/
void save_ppm( screen s, char *file) {

  int x, y;
  FILE *f;

  f = fopen(file, "w");
  fprintf(f, "P3\n%d %d\n%d\n", XRES, YRES, MAX_COLOR);
  for ( y=0; y < YRES; y++ ) {
    for ( x=0; x < XRES; x++)

      fprintf(f, "%d %d %d ", s[x][y].red, s[x][y].green, s[x][y].blue);
    fprintf(f, "\n");
  }
  fclose(f);
}

/*======== void save_extension() ==========
Inputs:   screen s
         char *file
Returns:
Saves the screen stored in s to the filename represented
by file.
If the extension for file is an image format supported
by the "convert" command, the image will be saved in
that format.
====================*/
void save_extension( screen s, char *file) {

  int x, y;
  FILE *f;
  char line[256];

  sprintf(line, "convert - %s", file);

  f = popen(line, "w");
  fprintf(f, "P3\n%d %d\n%d\n", XRES, YRES, MAX_COLOR);
  for ( y=0; y < YRES; y++ ) {
    for ( x=0; x < XRES; x++)

      fprintf(f, "%d %d %d ", s[x][y].red, s[x][y].green, s[x][y].blue);
    fprintf(f, "\n");
  }
  pclose(f);
}


/*======== void display() ==========
Inputs:   screen s
Returns:
Will display the screen s on your monitor
====================*/
void display( screen s) {

  int x, y;
  FILE *f;

  f = popen("display", "w");

  fprintf(f, "P3\n%d %d\n%d\n", XRES, YRES, MAX_COLOR);
  for ( y=0; y < YRES; y++ ) {
    for ( x=0; x < XRES; x++)

      fprintf(f, "%d %d %d ", s[x][y].red, s[x][y].green, s[x][y].blue);
    fprintf(f, "\n");
  }
  pclose(f);
}

void make_animation( char * name ) {

  int e, f;
  char name_arg[128];

  sprintf(name_arg, "anim/%s*", name);
  strncat(name, ".gif", 128);
  printf("Making animation: %s\n", name);
  f = fork();
  if (f == 0) {
    e = execlp("convert", "convert", "-delay", "3", name_arg, name, NULL);
    printf("e: %d errno: %d: %s\n", e, errno, strerror(errno));
  }
}
