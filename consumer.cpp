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

using namespace std;

int main(int argc, char* argv[])
{
  // Get the Index to the array assigned to this
  // consumer to process
  if(argc < 2)
  {
    cout << "Args: " << argv[0] << endl;
    perror("Consumer: Incorrect argument found");
    exit(EXIT_FAILURE);
  }
  // We can now get the index to the item to process
  const int nItemToProcess = atoi(argv[1]);

  // And the log file string
  string strLogFile = argv[2];

  // Seed the randomizer
  srand(time(NULL));

  // Register SIGQUIT handling
  signal(SIGINT, sigQuitHandler);

  // Log startup of the child
  const pid_t nPid = getpid();
  string strLog = "Consumer: PID ";
  strLog.append(GetStringFromInt(nPid));
  strLog.append(" Started");
  WriteLogFile(strLog, strLogFile);

  // Find the necessary Semaphores
  productSemaphores s(KEY_MUTEX, false);
  productSemaphores n(KEY_EMPTY, false);
  productSemaphores e(KEY_FULL, false);

  if(!s.isInitialized() || !n.isInitialized() || !e.isInitialized())
  {
    perror("Consumer: Could not successfully find Semaphores");
    exit(EXIT_FAILURE);
  }
  
  // Open the connection to shared memory
  // Allocate the shared memory
  // And get ready for read/write
  // Get a reference to the shared memory, if available
  shm_id = shmget(KEY_SHMEM, 0, 0);
  if (shm_id == -1) {
      perror("Consumer: Could not successfully find Shared Memory");
      exit(EXIT_FAILURE);
  }

  // Read the memory size and calculate the array size
  struct shmid_ds shmid_ds;
  shmctl(shm_id, IPC_STAT, &shmid_ds);
  size_t realSize = shmid_ds.shm_segsz;

  // Now we have the size - actually setup with shmget
  shm_id = shmget(KEY_SHMEM, realSize, 0);
  if (shm_id == -1) {
      perror("Consumer: Could not successfully find Shared Memory");
      exit(EXIT_FAILURE);
  }

  // attach the shared memory segment to our process's address space
  shm_addr = (char*)shmat(shm_id, NULL, 0);
  if (!shm_addr) { /* operation failed. */
      perror("Consumer: Could not successfully attach Shared Memory");
      exit(EXIT_FAILURE);
  }

  // Get the queue header
  struct ProductHeader* productHeader = 
      (struct ProductHeader*) (shm_addr);
  // Get our entire queue
  struct ProductItem*productItemQueue = 
      (struct ProductItem*) (shm_addr+sizeof(int)+sizeof(productHeader));

  // Get a random time to sleep between 1-10 seconds
  int nSleepTime = rand()%10+1;

  // Sleep for the random time
  while(!sigQuitFlag && !nSleepTime)
  {
    sleep(nSleepTime);
    nSleepTime--;
  }

  // Get Exclusive Access via Semaphores
  n.Wait();
  s.Wait();

  // Debug print queue
  //for(int i=0;i<productHeader->QueueSize;i++ )
  //  cout << productItemQueue[i].itemValue << " ";
  //cout << endl;

  // Consume the value
  float fNewPiVal =
    productItemQueue[nItemToProcess].itemValue;

  // Reset values
  productItemQueue[nItemToProcess].itemValue = 0.0f;
  productItemQueue[nItemToProcess].readyToProcess = false;

  // Log what happened into System Log
  strLog = "Consumer: PID ";
  strLog.append(GetStringFromInt(nPid));
  strLog.append(" Consumed Item in Queue: ");
  strLog.append(GetStringFromInt(nItemToProcess));
  WriteLogFile(strLog, strLogFile);

  cout << "Consumer: " << nPid << " consumed item in queue: " << nItemToProcess << endl;

  s.Signal();
  e.Signal();

  // Just let Consumer successfully die after it's done
  return EXIT_SUCCESS;
}