/*
 *
 * gcc-ascii-mandlebrot.c
 *
 * Copyright(C) 2023   MEJT
 *
 * Plots a mandlebrot set using non standard ANSI escape seqences.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.   If not, see <http://www.gnu.org/licenses/>.
 *
 * https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set
 * https://stackoverflow.com/questions/9065669/
 * https://stackoverflow.com/questions/8959610
 *
 * 07 Oct 23   0.1      - Initial version - MT
 *                      - Increased number of iterations to 64 - MT
 *
 */

#include <stdio.h>                        /* fprintf(), etc. */
#include <stdlib.h>                       /* exit(), etc. */
#include <string.h>                       /* strlen(), etc */
#include <stdarg.h>                       /* va_start(), va_end(), etc */

#include <unistd.h>
#include <math.h>

#define  NAME           "gcc-ansi-mandlebrot"
#define  VERSION        "0.1"
#define  BUILD          "0001"
#define  AUTHOR         "MT"
#define  DATE           "26 Sep 23"

#define  True           0
#define  False          !True

#if defined(DEBUG)
#define debug(code) do {fprintf(stderr, "Debug\t: %s line : %d : ", \
            __FILE__, __LINE__); code;} while(0)
#else
#define debug(code)
#endif

#define  WIDTH 132
#define  HEIGHT 33

int i_window_left, i_window_top;          /* Location of the window's top-left corner - relative to parent window. */
int i_KeyEventCode;

char *s_display_name = "";                /* Just use the default display. */
char *s_title = NAME;                     /* Windows title */

void v_version() /* Display version information */
{
   fprintf(stdout, "%s: Version %s", NAME, VERSION);
   if (__DATE__[4] == ' ') fprintf(stdout, " 0"); else fprintf(stdout, " %c", __DATE__[4]);
   fprintf(stdout, "%c %c%c%c %s %s", __DATE__[5],
      __DATE__[0], __DATE__[1], __DATE__[2], &__DATE__[9], __TIME__ );
   fprintf(stdout, " (Build: %s)\n", BUILD );
}

void v_about() /* Display help text */
{
   fprintf(stdout, "Usage: %s [OPTION]...\n", NAME);
   fprintf(stdout, "Display Mandlebrot or Juila set using ASCII characters.\n\n");
   fprintf(stdout, "  -?, --help               display this help and exit\n");
   fprintf(stdout, "      --version            output version information and exit\n");
   exit(0);
}

void v_error(const char *s_format, ...) /* Print formatted error message and exit */
{
   va_list t_args;
   va_start(t_args, s_format);
   fprintf(stderr, "%s : ", __FILE__);
   vfprintf(stderr, s_format, t_args);
   va_end(t_args);
   exit(-1);
}

int hsv_to_rgb(unsigned char h, unsigned char s, unsigned char v)
{
   unsigned char i, m, p, q, t;

   if(s == 0)
   {
      return ((((v << 8) + v) << 8) + v);
   }
   if (s == v) s = v +1;
   /** h = s - (h - v); /** Invert colours */
   i = ((int)(h / 42.5) + 1) % 6;
   m = (h - (i * 42.5)) * 6;

   p = (v * (255 - s)) >> 8;
   q = (v * (255 - ((s * m) >> 8))) >> 8;
   t = (v * (255 - ((s * (255 - m)) >> 8))) >> 8;

   switch(i)
   {
   case 0:
      return ((((v << 8) + p) << 8) + q);
      break;
   case 1:
      return ((((v << 8) + t) << 8) + p);
      break;
   case 2:
      return ((((q << 8) + v) << 8) + p);
      break;
   case 3:
      return ((((p << 8) + v) << 8) + t);
      break;
   case 4:
      return ((((p << 8) + q) << 8) + v);
      break;
   default:
      return ((((t << 8) + p) << 8) + v);
      break;
   }
}

int v_draw_mandlebrot_set()
{
   const float f_xmin = -2.25;            /* Left edge      */
   const float f_xmax = 0.75;             /* Right edge     */
   const float f_ymin = -1.25;            /* Top edge       */
   const float f_ymax = 1.25;             /* Bottom edge    */
   const int i_maxiteration = 64;         /* Iterations     */

   float f_xdelta = 0.00375;              /* X step size    */
   float f_ydelta = 0.0033;               /* Y step size    */

   float cr, ci;
   float zr, zi, temp;
   float r = 2.0;                         /* Radius         */
   float x, y;

   int i;
   int i_colour;

   f_xdelta = (f_xmin - f_xmax) / WIDTH;
   f_ydelta = (f_ymin - f_ymax) / HEIGHT;

   for (y = 0; y < HEIGHT; y++)
   {
      for (x = 0; x < WIDTH; x++)
      {
         cr = f_xmin - (x * f_xdelta);
         ci = f_ymin - (y * f_ydelta);
         zr = 0.0;
         zi = 0.0;
         i = 0;
         while ((((zr*zr) + (zi*zi)) < r*r) && (i < i_maxiteration))
         {
            temp = zr*zr - zi*zi;
            zi = 2 * zr * zi + ci;
            zr = temp + cr;
            i++;
         }
         if (i == i_maxiteration)
            printf("\033[48;2;0;0;0m ");
         else
         {
            i_colour = hsv_to_rgb(255 * ((float)i / i_maxiteration) , 255, 128);
            printf("\033[48;2;%03d;%03d;%03dm ", (i_colour >> 16)  & 0xff, (i_colour >> 8)  & 0xff, i_colour & 0xff);
         }
      }
      printf("\033[0m\n");
   }
   return True;
}

int main(int argc, char *argv[])
{
   int i_count, i_index;
   char b_abort = False; /* Stop processing command line */

   for (i_count = 1; i_count < argc && (b_abort != True); i_count++)
   {
      if (argv[i_count][0] == '-')
      {
         i_index = 1;
         while (argv[i_count][i_index] != 0)
         {
            switch (argv[i_count][i_index])
            {
               case '?': /* Display help */
                  v_about();
               case '-': /* '--' terminates command line processing */
                  i_index = strlen(argv[i_count]);
                  if (i_index == 2)
                    b_abort = True; /* '--' terminates command line processing */
                  else
                  {
                     if (!strncmp(argv[i_count], "--version", i_index))
                     {
                        v_version(); /* Display version information */
                        exit(0);
                     }
                     else if (!strncmp(argv[i_count], "--help", i_index))
                     {
                        v_about();
                     }
                     else /* If we get here then the we have an invalid long option */
                     {
                        v_error("%s: invalid option %s\nTry '%s --help' for more information.\n", argv[i_count][i_index] , NAME);
                        exit(-1);
                     }
                  }
                  i_index--; /* Leave index pointing at end of string (so argv[i_count][i_index] = 0) */
                  break;
               default: /* If we get here the single letter option is unknown */
                  v_error("unknown option -- %c\nTry '%s --help' for more information.\n", argv[i_count][i_index] , NAME);
                  exit(-1);
            }
            i_index++; /* Parse next letter in options */
         }
         if (argv[i_count][1] != 0) {
            for (i_index = i_count; i_index < argc - 1; i_index++) argv[i_index] = argv[i_index + 1];
            argc--; i_count--;
         }
      }
   }

   v_draw_mandlebrot_set(); /* Exit if unable to draw julia set */
   exit(0);
}
