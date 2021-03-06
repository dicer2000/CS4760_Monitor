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
#include "productSemaphores.h"
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
  // Seed the randomizer
  srand(time(NULL));

  // Register SIGQUIT handling
  signal(SIGINT, sigQuitHandler);

  int turn = 0;   // Used for Critical Section

  // Find the necessary Semaphores
  productSemaphores s(KEY_MUTEX, false);
  productSemaphores n(KEY_EMPTY, false);
  productSemaphores e(KEY_FULL, false);

  if(!s.isInitialized() || !n.isInitialized() || !e.isInitialized())
  {
    perror("Producer: Could not successfully find Semaphores");
    exit(EXIT_FAILURE);
  }


// Open the connection to shared memory
  // Allocate the shared memory
  // And get ready for read/write
  // Get a reference to the shared memory, if available
  shm_id = shmget(KEY_SHMEM, 0, 0);
  if (shm_id == -1) {
      perror("shmget1: ");
      exit(EXIT_FAILURE);
  }

  // Read the memory size and calculate the array size
  struct shmid_ds shmid_ds;
  shmctl(shm_id, IPC_STAT, &shmid_ds);
  size_t realSize = shmid_ds.shm_segsz;

  // Now we have the size - actually setup with shmget
  shm_id = shmget(KEY_SHMEM, realSize, 0);
  if (shm_id == -1) {
      perror("shmget2: ");
      exit(EXIT_FAILURE);
  }

  // attach the shared memory segment to our process's address space
  shm_addr = (char*)shmat(shm_id, NULL, 0);
  if (!shm_addr) { /* operation failed. */
      perror("shmat: ");
      exit(EXIT_FAILURE);
  }

  // Get the queue header
  struct ProductHeader* productHeader = 
      (struct ProductHeader*) (shm_addr);
  // Get our entire queue
  struct ProductItem*productItemQueue = 
      (struct ProductItem*) (shm_addr+sizeof(int)+sizeof(productHeader));

  // Loop until signaled to shutdown via SIGINT
  while(!sigQuitFlag)
  {
    // Get a random time to sleep between 1-10 seconds
    int nSleepTime = rand()%10+1;

    // Sleep for the random time
    sleep(nSleepTime);

    // Get Exclusive Access via Semaphores
    n.Wait();
    s.Wait();

    // Is there a new item available in the Queue? If so, then
    // extract the value, and increment the pCurrent pointer
    if(productItemQueue[productHeader->pCurrent%productHeader->QueueSize].readyToProcess)
    {

        // Debug print queue
  for(int i=0;i<productHeader->QueueSize;i++ )
    cout << productItemQueue[i].itemValue << " ";
  cout << endl;
//  cout << "c-pCurrent:" << productHeader->pCurrent << endl;
//  cout << "c-pNextQueue:" << productHeader->pNextQueueItem << endl;
//  cout << "c-QueueSize:" << productHeader->QueueSize << endl;


      // Consume the value
      float fNewPiVal =
        productItemQueue[productHeader->pCurrent%productHeader->QueueSize].itemValue;

      // Reset value
      productItemQueue[productHeader->pCurrent%productHeader->QueueSize].itemValue = 0.0f;

      // Mark as consumed
      productItemQueue[productHeader->pCurrent%productHeader->QueueSize].readyToProcess = false;

      // Increment Current and wrap it around if > queue size
      productHeader->pCurrent = (++productHeader->pCurrent)%productHeader->QueueSize;

    }

    s.Signal();
    e.Signal();

  }

  return EXIT_SUCCESS;
}