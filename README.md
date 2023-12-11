# Serial Interface Program

I made this program to see how accessing a serial port works in C, and because I
wasn't sure if `screen` wrote to device ports. Imagine my surprise when I found 
out device interfaces just work like reading/writing to a normal file.

That said, it was a small exercise in remembering how to make a state machine
*and* it's technically my introduction to Pthreads/threading in general. It's a
very simple thread, to be sure, but this program does qualify as multithreaded!

Anyway, not really meant for anyone's use but mine, it speaks by default
(hard-coded) to `/dev/ttyACM0` because that's what the Nano BLE 33 and Pi Pico
use, and I can change it really easily.

Write functionality isn't actually tested yet, but I plan on making a shell-like 
program with FreeRTOS on the Pi Pico soon so I'll probably test it then.
