/********************************************
 * consumer - Semaphores and Message Passing
 * This file is for the consumer functionality of the
 * application.  It kicks off from the monitor process
 * 
 * Brett Huffman
 * CMP SCI 4760 - Project 3
 * Due Mar 12, 2021
 * consumer h file for project
 ********************************************/
#include <iostream>
#include <unistd.h>
#include "sharedStructures.h"
#include <fstream>

// Forward Declarations
static void show_usage(std::string);

// SIGQUIT handling
volatile sig_atomic_t sigQuitFlag = 0;
void sigQuitHandler(int sig){ // can be called asynchronously
  sigQuitFlag = 1; // set flag
}

// Critical Section Turn Flag
extern int turn;

const char* LogFile = "adder_log";

using namespace std;

int main(int argc, char* argv[])
{

    // Register SIGQUIT handling
    signal(SIGINT, sigQuitHandler);

    int turn = 0;   // Used for Critical Section
    


    return 0;
}