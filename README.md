# Facit N4000 Punch Simulator
This simulates a Facit N4000 punch, showing the punch holes for 8 channel raw data only.
FLTK is used to draw the punch tape, and you can scroll right and left to view a long tape.
FLTK is a cross platform C++ graphics toolkit available from fltk.org.

The application includes a sample file, "testpat.bin", used to punch a tape on an actual Facit N4000.
The image it generates matches the tape; I've been using this to test a program that punches tapes,
the goal to not waste paper tape during development to catch silly programming errors.

To use the tool, just have your raw Facit punch data in a file, and invoke the application with that file
as the single argument, e.g.

    ./punchtape yourdata.bin

Here's a screenshot of the application displaying the "testpat.bin" file:
![screenshot](https://github.com/erco77/facit-n4000-punch-simulator/blob/master/screenshot.png)

The raw data would be in the same format you'd send to the Facit over its serial port to punch a tape.
I was only interested in generated 8 channel punch tapes, as that's what the project I'm working on needs.
This doesn't handle any of the Facit's "special codes"; it considers all 8 bits in each byte
of data as valid punch hole pattern values.

Feel free to fork the project to support other punch formats the Facit N4000 supports.
If the code looks simple and straight forward I might merge.

This app was quickly written, but there's not much to it; it just draws the punch circles
in the right places, simulating the tape.

Written and tested on Linux, but the same Makefile should work for mac os.
For windows, not so much; a separate Makefile would be needed to build for Visual Studio or mingw/msys.

## Keyboard/Mouse
There's a scrollbar at the bottom of the tape that lets you scroll left/right with the mouse to view long tapes.
Also the left/right arrow keys can be used to scroll through the tape.
