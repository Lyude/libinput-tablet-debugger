libinput-tablet-debugger
========================

A simple tool for viewing the values [libinput](http://www.freedesktop.org/wiki/Software/libinput/) returns for a wacom tablet. This tool might not always work due to the fact that the API for working with tablets in libinput has not been finalized yet, along with the fact the API for everything else in libinput has not completely been finalized either.

How to build
----------
Make sure you have the following dependencies installed:
- cmake
- udev
- ncurses with the panel library included
- udev
- [My fork of libinput](https://github.com/Lyude1337/libinput/tree/carlos_cleanup)
  As of writing, this debugger should at least work with [this commit](https://github.com/Lyude1337/libinput/tree/ee0f095af245fe3f143c6342b36edc9db1f67dd6), later versions may work but are not guaranteed

Building should be as simple as going into the build directory and running the following commands:
```
cmake .
make
```

How to use
----------
Run the `li-tablet-debug` binary in the `src/` directory as root. If you have a wacom tablet connected, various fields with values from the tablet should be displayed. If you have more then one tablet connected, you can navigate between each tablet by using the left and right keys on your keyboard.
To exit the program, press `q`.
