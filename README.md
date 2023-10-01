## x11-julia - Display a Julia or Mandlebrot set in a Window

![Screenshot](./x11-julia.png)![Screenshot](./x11-mandlebrot.png)

Two seperate X11 programs to display either a Julia Set or a Mandlebrot set
in a window or fullscreen.

Written in standard C using X11.  The use of any language extensions or non
standard language features has been avoided.

The  aim  is to have the same source code compile without  modification  on
Linux, VAX/VMS, and Tru64 Unix.


### Compiling

To  build the application on Linux make sure that you have all the required
prerequisites installed then download the source code from GitHub and unzip
it  (a  new folder  will  be created to automatically).

Then change directory to the new  folder run 'make'.

e.g:

    $ wget https://github.com/mike632t/x11-julia/archive/refs/heads/stable.zip
    $ unzip stable.zip
    $ cd x11-julia-stable
    $ make

If more than one C compiler is installed then you can specify which one to use  on
the command line.

    $ make CC=tcc

    $ make CC=clang

Note - 'CC' is in UPPERCASE.


### Keyboard Shortcuts

'Escape' quits,  and  'F' or 'f' toggle the full-screen display.


### Exiting

To quit just press 'Escape' or close the window.


### Known Issues

A 24 bit colour display is required.

Not tested on VMS.


### Tested

- Debian 10 (Buster), gcc 8.3.0, x64

- Debian 10 (Buster), clang 7.0.1, x64

- SUSE 15.4, clang 13. 0.1, x64

- SUSE 15.4, gcc 7.5.0, x64

- Ubuntu 20.04, gcc 9.4.0, x64

- Ubuntu 20.04, clang 10.0.0, x64

- Ubuntu 20.04, tcc 0.9.27, x64


### Prerequisites

The following packages are required to build the application

- Debian : gcc | clang | tcc, make, libx11-dev, libc6-dev

- Fedora : gcc, make, libx11-dev, libc6-dev

- Gentoo : gcc, make, libx11-dev, libc6-dev

- MacOS  : clang, make, [xquartz](https://www.xquartz.org/)

- SUSE   : gcc | clang, make, libX11-devel

- Ubuntu : gcc, make, libx11-dev, libc6-dev


### Problem Reports

If you find problems or have suggestions relating to these simulators, then
please create a new [issue](https://github.com/mike632t/x11-julia/issues).

Your problem report should contain:

- Architecture (and VM host if applicable);

- Operating System and version;

- Desktop Environment and version;

- Window Manager and version;

- Compiler and version used;

- Commit ID;

- A description of the problem.

Thank you.

