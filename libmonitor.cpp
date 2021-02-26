/********************************************
 * monitor - Semaphores and Message Passing
 * This file is for the master functionality of the
 * application.  It is implemented as a seperate
 * object file that is statically linked with the
 * monitor main application.
 * 
 * Brett Huffman
 * CMP SCI 4760 - Project 3
 * Due Mar 12, 2021
 * Monitor CPP file for project
 ********************************************/
#include <iostream>
#include <string.h>
#include <vector>


#include "sharedStructures.h"

// Static process counter => Never > 20 (1 Parent + 19 Children)
const int MAX_PROCESSES = 19;
static int ProcessCount = 0;
const int BUFFERSIZE = 8192;

using namespace std;

// Item Variables
vector<int> vecItemArray;
int* addItem_num;
struct AddItem* addItems;

// SIGINT handling
volatile sig_atomic_t sigIntFlag = 0;
void sigintHandler(int sig){ // can be called asynchronously
  sigIntFlag = 1; // set flag
}
