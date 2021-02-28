# monitor Application

monitor application (Semaphores and Message Passing) by Brett Huffman for CMP SCI 4760 - Project 3 - V1.0

This program implements the Semaphores and Message Passing program.  It implements the concept of a monitor, provided as a statically linked library, and use it to solve the problem.  It implements semaphores and solves the Producer-Consumer problem with monitors.

A log was kept of each day's activity.  It is found at the end of this README file.

A Git repository was maintained with a public remote found here: https://github.com/dicer2000/CS4760_Monitor.git

## Program Switches
The program can be invoked as:

```
monitor -h
monitor [-o logfile] [-p m] [-c n] [-t time]
Options:
  -h Describe how the project should be run and then, terminate.
  -o logfile Name of the file to save logs; default: logfile
  -p m Number of producers; default: m = 2
  -c n Number of consumers; default: n = 6
  -t time The time in seconds after which the process will terminate, even if it has not finished. (Default: 100)
```

## Install
To install this program, clone it with git to the folder to which you want 
it saved.
```
git clone https://github.com/dicer2000/CS4760_Monitor.git
```
## Compile
To compile the master application, simply run the make command:
```
make
```
## Run
To run the program, use the master command.  You can use any of the attributes listed in program switches area.

## Problems / Issues

Well, the biggest day-one issue was getting a static library to work with make and the main application.  It turns out it's pretty easy... just a small learning curve to some of this other stuff.

Monitors are now the big question of the day.  I'm reviewing the lecture and also some online resources.  My question is should it be implemented all within the monitor?  Or maybe it should be partially in the monitor and then also in the consumer.  Or maybe the consumer calls the monitor to lock?  I'm not sure how it works together.


## Work Log

- 2/25/2021 - Created project, Makefile, this readme file and got it all to compile; Built the static monitor library and got it to compile and work with make.
- 2/26/2021 - Researched monitors; Started working up how it will integrated
- 2/27/2021 - Added monitors and shared memory

*©2021 Brett W. Huffman*