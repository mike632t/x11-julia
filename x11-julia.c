/*
 *
 * x11-julia.c
 *
 * Copyright(C) 2023   MEJT
 *
 * Plots a julia set and waits for the user to press any key to exit.
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
 * 16 Sep 23   0.1      - Initial version - MT
 * 30 Sep 23   0.2      - Added ability to parse command line options - MT
 * 07 Oct 23            - Changed the way the colour is determined allowing
 *                        the number of iterations to be more then 255 - MT
 *
 * To Do                - Pass coefficents from the command line?
 *
 */

#include <stdio.h>                        /* fprintf(), etc. */
#include <stdlib.h>                       /* exit(), etc. */
#include <string.h>                       /* strlen(), etc */
#include <stdarg.h>                       /* va_start(), va_end(), etc */

#include <unistd.h>
#include <math.h>

#include <X11/Xlib.h>                     /* XOpenDisplay(), etc. */
#include <X11/Xatom.h>                    /* XA_ATOM */
#include <X11/keysym.h>

#define  NAME           "x11-julia"
#define  VERSION        "0.1"
#define  BUILD          "0001"
#define  AUTHOR         "MT"
#define  DATE           "26 Sep 23"

#if defined(DEBUG)
#define debug(code) do {fprintf(stderr, "Debug\t: %s line : %d : ", \
            __FILE__, __LINE__); code;} while(0)
#else
#define debug(code)
#endif

#define  WIDTH 800                        /* Define window size */
#define  HEIGHT 600

Display *h_display;                       /* Pointer to X display structure. */
Window x_application_window;              /* Application window structure. */
Window x_root_window;                     /* Root window structure. */
XEvent x_event;
Atom wm_state;
Atom wm_fullscreen;

unsigned int i_screen;                    /* Default screen number */
unsigned int i_window_width = WIDTH ;     /* Window width in pixels. */
unsigned int i_window_height = HEIGHT ;   /* Window height in pixels. */
unsigned int i_window_border = 4 ;        /* Window's border width. */
unsigned int i_background_colour;         /* Window's background colour. */
unsigned int i_colour_depth;              /* Window's colour depth. */

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
   fprintf(stdout, "Display Mandlebrot or Juila set.\n\n");
   fprintf(stdout, "  -f, --fullscreen         display in fullscreen window\n");
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

void v_set_blank_cursor(Display *x_display, Window x_application_window, Cursor *x_cursor)
{
   Pixmap x_blank;
   XColor x_Color;
   char c_pixmap_data[1] = {0}; /* An empty pixmap */
   x_blank = XCreateBitmapFromData (x_display, x_application_window, c_pixmap_data, 1, 1); /* Create an empty bitmap */
   (*x_cursor) = XCreatePixmapCursor(x_display, x_blank, x_blank, &x_Color, &x_Color, 0, 0); /* Use the empty pixmap to create a blank cursor */
   XFreePixmap (x_display, x_blank); /* Free up pixmap */
}

void v_fullscreen(int i_State)
{
   XEvent x_event;
   Atom wm_fullscreen;

   x_event.type = ClientMessage;
   x_event.xclient.window = x_application_window;
   x_event.xclient.message_type = XInternAtom(h_display, "_NET_WM_STATE", False);
   x_event.xclient.format = 32;
   x_event.xclient.data.l[0] = i_State;
   wm_fullscreen = XInternAtom(h_display, "_NET_WM_STATE_FULLSCREEN", False);
   x_event.xclient.data.l[1] = wm_fullscreen;
   x_event.xclient.data.l[2] = wm_fullscreen;
   XSendEvent(h_display, RootWindow(h_display, i_screen), False, ClientMessage, &x_event);
}

int hsv_to_rgb(unsigned char h, unsigned char s, unsigned char v)
{
   unsigned char i, m, p, q, t;

   if(s == 0)
   {
      return ((((v << 8) + v) << 8) + v);
   }
   if (s == v) s = v +1;
   /**h = s - (h - v); /* Invert colours */
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

int v_draw_julia_set(float cr, float ci)
{
   const float f_xmin = -1.55;            /* Left edge      */
   const float f_xmax = 1.55;             /* Right edge     */
   const float f_ymin = -0.9;             /* Top edge       */
   const float f_ymax = 0.9;              /* Bottom edge    */
   const int i_maxiteration = 224;        /* Iterations     */

   float f_xdelta = 0.00375;              /* X step size    */
   float f_ydelta = 0.0033;               /* Y step size    */

   float zr, zi, temp;
   float r = 2.0;                         /* Radius         */
   float x, y;

   int i_colour;
   int i;

   /* Get window geometry - not everything will always be the same as the
      values we requested for when we created the window - particularly if
      it has been resized! */

   if (XGetGeometry(h_display, x_application_window,
         &RootWindow(h_display, i_screen),
         &i_window_left, &i_window_top,
         &i_window_width,
         &i_window_height,
         &i_window_border,
         &i_colour_depth) == False)
   {
      return (False);
   }

   f_xdelta = (f_xmin - f_xmax) / i_window_width;
   f_ydelta = (f_ymin - f_ymax) / i_window_height;
   for (y = 0; y < i_window_height; y++)
   {
      for (x = 0; x < i_window_width; x++)
      {
         zr = f_xmin - (x * f_xdelta);
         zi = f_ymin - (y * f_ydelta);
         i = 0;
         while ((((zr*zr) + (zi*zi)) < r*r) && (i < i_maxiteration))
         {
            temp = zr*zr - zi*zi;
            zi = 2 * zr * zi + ci;
            zr = temp + cr;
            i++;
         }
         i_colour = hsv_to_rgb(255 * ((float)i / i_maxiteration) , 255, 128);
         if (i == i_maxiteration)
            XSetForeground(h_display, DefaultGC(h_display, i_screen), BlackPixel(h_display, i_screen));
         else
            XSetForeground(h_display, DefaultGC(h_display, i_screen), i_colour);
         XDrawPoint(h_display, x_application_window, DefaultGC(h_display, i_screen), x, y);
         XFlush(h_display);
      }
   }
   return True;
}

int main(int argc, char *argv[])
{
   int i_count, i_index;
   int b_fullscreen = False;
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
               case 'f': /* Fullscreen */
                   b_fullscreen = True; break;
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
                     else if (!strncmp(argv[i_count], "--fullscreen", i_index))
                     {
                        b_fullscreen = True;
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

   h_display = XOpenDisplay(s_display_name); /*   Open a display. */

   if (h_display) /*   If successful create and display a new window. */
   {
      x_root_window = DefaultRootWindow(h_display); /* Get the ID of the root window of the screen. */
      i_screen = DefaultScreen(h_display); /* Get the default screen for our X server. */

      x_application_window = XCreateSimpleWindow(h_display, RootWindow(h_display, i_screen), /* Create the application window, as a child of the root window. */
         i_window_width, i_window_height, /* Window position -igore ? */
         i_window_width, /* Window width */
         i_window_height, /* Window height */
         i_window_border, /* Border width - ignored ? */
         BlackPixel(h_display, i_screen), /* Preferred method to set border colour to black */
         WhitePixel(h_display, i_screen)); /* Preferred method to set background colour to white */

      if (b_fullscreen)
      {
         /*
          *  Display julia set in full screen maode by default
          *
          */
         Atom wm_state = XInternAtom (h_display, "_NET_WM_STATE", True );
         Atom wm_fullscreen = XInternAtom (h_display, "_NET_WM_STATE_FULLSCREEN", True );
         if (XChangeProperty(h_display, x_application_window, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *)&wm_fullscreen, 1) );
      }

      XStoreName(h_display, x_application_window, s_title); /* Set the window title */

      XSync(h_display, False); /* Flush display before drawing (showing) the window! */
      XMapWindow(h_display, x_application_window); /*   Show the window */
      XSelectInput(h_display, x_application_window, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);  /* Select events we are interested in, note ButtonPress is required for ButtonRelease */

      while (!b_abort)
      {
         XNextEvent(h_display, &x_event); /* Get next windows event */
         switch (x_event.type)
         {
         case Expose: /* Draw or redraw the window */
            b_abort = !v_draw_julia_set(-0.79, 0.15); /* Try (-0.79, 0.15), (-0.75, 0.11) or (-0.74543, 0.11301) */
            break;
         case ButtonRelease:
            break;
         case KeyPress:
            switch (XLookupKeysym(&x_event.xkey, 0))
            {
            case XK_Escape:
               b_abort = True; /* Exit if a key is pressed */
               break;
            case XK_f:
            case XK_F:
               if (b_fullscreen)
                  v_fullscreen(0);
               else
                  v_fullscreen(1);
               b_fullscreen = !b_fullscreen;
            }
         }
      }
      /* Close connection to server */
      XCloseDisplay(h_display);
   }
   else {
      /* fprintf(stderr, "%s: cannot connect to X server '%s'\n", argv[0], s_display_name); */
      v_error("Cannot connect to X server '%s'\n", s_display_name);
   }
   exit(0);
}
