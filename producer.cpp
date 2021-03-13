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
#include <fstream>
#include "sharedStructures.h"
#include "productSemaphores.h"

// Forward Declarations
static void show_usage(std::string);

// SIGQUIT handling
volatile sig_atomic_t sigQuitFlag = 0;
void sigQuitHandler(int sig){ // can be called asynchronously
  sigQuitFlag = 1; // set flag
}

using namespace std;

int main(int argc, char* argv[])
{
  // Fast Seed Random for better performance
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);

  // Using nano-seconds instead of seconds
  srand((time_t)ts.tv_nsec);

  // Register SIGQUIT handling
  signal(SIGINT, sigQuitHandler);

  int childPid = getpid(); // ProducerID to log
  
  // Log startup of the child
  string strLog = "Producer PID ";
  strLog.append(GetStringFromInt(childPid));
  strLog.append(" Started");
  WriteLogFile(strLog);

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
        perror("Producer: Could not successfully find Shared Memory");
        exit(EXIT_FAILURE);
    }

    // Read the memory size and calculate the array size
    struct shmid_ds shmid_ds;
    shmctl(shm_id, IPC_STAT, &shmid_ds);
    size_t realSize = shmid_ds.shm_segsz;
//    int length = (int) shmid_ds.shm_segsz / sizeof(AddItem);

    // Now we have the size - actually setup with shmget
    shm_id = shmget(KEY_SHMEM, realSize, 0);
    if (shm_id == -1) {
        perror("Producer: Could not successfully find Shared Memory");
        exit(EXIT_FAILURE);
    }

    // attach the shared memory segment to our process's address space
    shm_addr = (char*)shmat(shm_id, NULL, 0);
    if (!shm_addr) { /* operation failed. */
        perror("Producer: Could not successfully attach Shared Memory");
        exit(EXIT_FAILURE);
    }

    // Get the queue header
    struct ProductHeader* productHeader = 
        (struct ProductHeader*) (shm_addr);
    // Get our queue right after the header
    struct ProductItem*productItemQueue = 
        (struct ProductItem*) (shm_addr+sizeof(int)+sizeof(productHeader));

  // Loop until signaled to shutdown via SIGINT
  while(!sigQuitFlag)
  {
    // Get a random time to sleep between 1-5 seconds
    int nSleepTime = rand()%5+1;

    // Sleep for my random time
    sleep(nSleepTime);

    // The productHeader->pNextQueueItem => Next one to put new product in
    // the productHeader->pCurrent => Next one to consume

    // Produce an item by putting a number on the queue
    // As a little easter egg, since this is due pretty
    // close to PI day, I'm going to (loosly) calcuate
    // pi and return it as my product
    float fEasterEgg = 355.0f/113.0f;

    // Get Exclusive Access via Semaphores
    e.Wait();
    s.Wait();

    // Push this onto the Queue
    productItemQueue[productHeader->pNextQueueItem].itemValue = fEasterEgg;

    // Mark as ready to be process
    productItemQueue[productHeader->pNextQueueItem].readyToProcess = true;

    // Log what happened into System Log
    string strLog = GetStringFromInt(childPid);
    strLog.append(" Produced Item in Queue: ");
    strLog.append(GetStringFromInt(productHeader->pNextQueueItem));
    WriteLogFile(strLog);

    cout << "Producer: " << childPid << " added item to Queue: " << productHeader->pNextQueueItem << endl;

    // Add an item to the next queue and wrap it around if it's > queue size
    productHeader->pNextQueueItem = (++productHeader->pNextQueueItem)%productHeader->QueueSize;

  // Debug print queue
  //cout << "p-pCurrent:" << productHeader->pCurrent << endl;
  //cout << "p-pNextQueue:" << productHeader->pNextQueueItem << endl;
  //cout << "p-QueueSize:" << productHeader->QueueSize << endl;
  //  for(int i=0;i<productHeader->QueueSize;i++ )
  //    cout << productItemQueue[i].itemValue << " ";
  //  cout << endl;

    s.Signal();
    n.Signal();
  }

    return 0;
}