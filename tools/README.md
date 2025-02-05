## Tools

This directory contains auxiliary tools and files.


## send_file

`send_file` is simple example script for sending multiple lines
from a file to the serial link.

Compile bundled `usleep.c` before executing the `send_file` script
(example compilation command is included in `usleep.c` file).

`send_file` is expected to be executed from the command line while
other serial communication program is started, so the output from
the board can be seen (tested with `putty`).

Default line delay is set to 4ms which seems to be sufficient
for WozMon processing, but can be adjusted in `send_file` if
needed.

Note that correct serial port should be set in `send_file` script.

Tested on sending programs from "Apple-1-Mini" repository
https://github.com/DutchMaker/Apple-1-Mini/tree/master/code/programs


## hello_world.txt

`hello_world.txt` is example of data input for simple asm program.

It expects the board is in WozMon. It sets program and data in
memory, lists memory contents and runs the program.

Send to board with command

    send_file hello_world.txt

Note that correct serial port should be set in `send_file` script.

You can disassemble the code using Krusader by entering (starting from WozMon)

    F000R
    D $1000

after which you should see

    F000R
    
    F000: A9
    KRUSADER 1.2 BY KEN WESSEN
    ? D $1000
    1000   A2 00       LDX   #$00
    1002   BD 13 10    LDA   $1013,X
    1005   20 EF FF    JSR   $FFEF
    1008   E8          INX
    1009   E0 15       CPX   #$15
    100B   F0 03       BEQ   $1010
    100D   4C 02 10    JMP   $1002
    1010   4C 1F FF    JMP   $FF1F
    1013   8D 8D 8D    STA   $8D8D
    1016   48          PHA
    1017   45 4C       EOR   $4C
    1019   4C 4F 20    JMP   $204F
    101C   57          STY
    101D   4F          STY
    101E   52          STY
    101F   4C 44 20    JMP   $2044
    1022   21 21       AND   ($21,X)
    1024   21 8D       AND   ($8D,X)
    1026   8D 8D 7D    STA   $7D8D
    1029   E7          STY
    ?



## basic_test.txt

`basic_test.txt` is example of data input for basic BASIC test.

It expects the board is in WozMon. It starts Integer Basic, enters
simple program, list it and run it.

Send to board with command

    send_file basic_test.txt

Note that correct serial port should be set in `send_file` script.


## usleep.c

Source code for microsecond sleep utility used by `send_file`.
Added here for completeness. 

Needs to be compiled. Example compilation command is included
in the file.

