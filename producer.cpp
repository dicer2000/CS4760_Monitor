/********************************************
 * producer - Semaphores and Message Passing
 * This file is for the producer functionality of the
 * application.  It kicks off from the monitor process
 * 
 * Brett Huffman
 * CMP SCI 4760 - Project 3
 * Due Mar 12, 2021
 * producer CPP file for project
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

using namespace std;

int main(int argc, char* argv[])
{
  // Randomize the seed
  srand(time(NULL));

  // Register SIGQUIT handling
  signal(SIGINT, sigQuitHandler);

  int turn = 0;   // Used for Critical Section
  int myPID = getpid(); // ProducerID to log
    
  // Loop until signaled to shutdown via SIGINT
  while(!sigQuitFlag)
  {
    // Get a random time to sleep between 1-5 seconds
    int nSleepTime = rand()%5+1;

    // Sleep for my random time
    sleep(nSleepTime);

    // Produce an item by putting a number on the queue

    // As a little easter egg, since this is due pretty
    // close to PI day, I'm going to (loosly) calcuate
    // pi and return it as my product
    float fEasterEgg = 355.0f/113.0f;

    // Push this onto the Queue

    // Log what happened into System Log
    
  }


    return 0;
}