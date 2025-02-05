# PIA Communicator NG

This is rewritten version of "PIA Communicator" for RC6502 Apple 1 SBC.

Based on original source code:
https://github.com/tebl/RC6502-Apple-1-Replica/tree/master/software/arduino/PIA%20Communicator.


## Goal

Main goal is to fix deficiencies of original design, especially 
- wildly outputting characters while RESET switch is pressed on SBC
- lost characters in the output (dump of 256 bytes in WozMon contained several missing characters)

Further goal is to correctly run TEST PROGRAM from "Apple-1 Operation Manual" (pg.2).



## HW Changes

- Shorten C12/10nF (or remove and replace with a wire)
- Route RESET signal to pin 9 of Arduino Nano (e.g. from pin 4 of 7404)



## Build

Open project directory in VSCode, PlatformIO: Build, PlatformIO: Upload.



## Results

All goals were achieved.

PIA video and keyboard control signals were checked with oscilloscope
with satisfactory results.

Output was tested on continuous run of TEST PROGRAM from
"Apple-1 Operation Manual". Megabyte of data was received without
a single character loss.


NOTE:
While I think it should work without any problem, I have to mention
that I haven't tested the code with original RESET circuit.
I was so unhappy with RESET circuit functionality that I've replaced 555
timer with PIC12C509A.



## Test Program

TEST PROGRAM from "Apple-1 Operation Manual" (pg.2).

Program code:

    0: a9 0 AA 20 EF FF E8 8A 4C 2 0

Check the memory

    0.a

Run

    0r


## Links

There is a collection of programs in "Apple-1-Mini" repository
https://github.com/DutchMaker/Apple-1-Mini/tree/master/code/programs


