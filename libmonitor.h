/********************************************
 * monitor.h - Master include file
 * 
 * Brett Huffman
 * CMP SCI 4760 - Project 3
 * Due Mar 12, 2021
 * Master .h file for project
 ********************************************/
#ifndef LIBMONITOR
#define LIBMONITOR

#include <iostream>

using namespace std;

// Function Declarations
int monitorProcess(string, int, int, int);
int forkProcess(string, int nArrayItem = -1);

#endif // LIBMONITOR