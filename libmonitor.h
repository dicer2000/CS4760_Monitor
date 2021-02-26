/********************************************
 * monitor.h - Master include file
 * 
 * Brett Huffman
 * CMP SCI 4760 - Project 3
 * Due Mar 12, 2021
 * Master .h file for project
 ********************************************/
#include <iostream>

using namespace std;

// Master Function Declarations
int processMonitor(int numberOfChildrenAllowed, int timeInSecondsToTerminate, string InputDataFile);
int forkProcess(int nItemStart, int nDepth);
