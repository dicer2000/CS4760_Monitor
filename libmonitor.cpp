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
#include <unistd.h>

#include "libmonitor.h"
#include "sharedStructures.h"
#include "productSemaphores.h"

// Static process counter => Never > 20 (1 Parent + 19 Children)
static int ProcessCount = 0;
const int MAX_PROCESSES = 19;
const int BUFFERSIZE = 8192;

using namespace std;

// Important Item queues + arrays
struct ProductHeader* productHeader;
struct ProductItem* productItemQueue;
vector<int> vecProducers;
vector<int> vecConsumers;

// SIGINT handling
volatile sig_atomic_t sigIntFlag = 0;
void sigintHandler(int sig){ // can be called asynchronously
  sigIntFlag = 1; // set flag
}



int monitorProcess(string InputDataFile, int nNumberOfProducers, int nNumberOfConsumers, int nSecondsToTerminate)
{
  // Check Input and exit if a param is bad
  if(InputDataFile.size() < 1 || nNumberOfProducers < 1 || 
    nNumberOfConsumers < 1 || nSecondsToTerminate < 1)
  {
    errno = EINVAL;
    perror("Unknown option");
    return EXIT_FAILURE;
  }

  // Register SIGINT handling
  signal(SIGINT, sigintHandler);
  bool isKilled = false;
  bool bComplete = false;

  // Start Time for time Analysis
  time_t secondsStart;
  // Get the time in seconds for our process to make
  // sure we don't exceed the max amount of processing time
  secondsStart = time(NULL);   // Start time

  // Create the necessary Semaphores with the
  // productSemaphores class
  productSemaphores s(KEY_MUTEX, true, 1);
  productSemaphores n(KEY_EMPTY, true, 0);
  productSemaphores e(KEY_FULL, true, PRODUCT_QUEUE_LENGTH);

  if(!s.isInitialized() || !n.isInitialized() || !e.isInitialized())
  {
    perror("Monitor: Could not successfully create Semaphores");
    exit(EXIT_FAILURE);
  }
  // Setup shared memory
  // allocate a shared memory segment with size of 
  // Product Header + entire Product array
  int memSize = sizeof(ProductHeader) + sizeof(ProductItem) * PRODUCT_QUEUE_LENGTH;
  shm_id = shmget(KEY_SHMEM, memSize, IPC_CREAT | IPC_EXCL | 0660);
  if (shm_id == -1) {
      perror("shmget: ");
      exit(EXIT_FAILURE);
  }

  // attach the shared memory segment to our process's address space
  shm_addr = (char*)shmat(shm_id, NULL, 0);
  if (!shm_addr) { /* operation failed. */
      perror("shmat: ");
      exit(EXIT_FAILURE);
  }
  // Get the queue header
  productHeader = (struct ProductHeader*) (shm_addr);
  // Get our entire queue
  productItemQueue = (struct ProductItem*) (shm_addr+sizeof(int)+sizeof(productHeader));

//  productItemQueue = (struct ProductItem*) (shm_addr+sizeof(int));
  // Fill the product header
  productHeader->pCurrent = 0;
  productHeader->pNextQueueItem = 0;
  productHeader->QueueSize = PRODUCT_QUEUE_LENGTH;

  // Set all items in queue to empty
  for(int i=0; i < PRODUCT_QUEUE_LENGTH; i++)
  {
      productItemQueue[i].readyToProcess = true;
      productItemQueue[i].itemValue = 0.0f;
  }
  
  // Start up producers by fork/exec nNumberOfProducers
  cout << "Starting Producers" << endl;
  for(int i=0; i < nNumberOfProducers; i++)
  {
    // Fork and store pid Producer Vector
    pid_t pid = forkProcess(ProducerProcess);
    if(pid > 0)
    {
      vecProducers.push_back(pid);
      cout << "Producer " << vecProducers[i] << " started" << endl;
    }
  }

  // Check that we actually have some producers
  if(vecProducers.size() < 1)
  {
    errno = ECANCELED;
    perror("Could not create Producers");
    isKilled = true;
  }

  // Start up Consumers
  cout << "Starting Consumers" << endl;
  for(int i=0; i < nNumberOfConsumers; i++)
  {
    // Fork and store pid Producer Vector
    pid_t pid = forkProcess(ConsumerProcess);
    if(pid > 0)
    {
      vecConsumers.push_back(pid);
      cout << "Consumer " << vecProducers[i] << " started" << endl;
    }
  }

  // Check that we actually have some producers
  if(vecConsumers.size() < 1)
  {
    errno = ECANCELED;
    perror("Could not create Consumers");
    isKilled = true;
  }

  // Keep track of waits & pids
  pid_t waitPID;
  int wstatus;

  // Loop until timeout or interrupt exit
  while(!isKilled && !sigIntFlag && !((time(NULL)-secondsStart) > nSecondsToTerminate))
  {
    // If any new products show up and < max processes are created,
    // create a new consumer to consume it
//    cout << productHeader->pNextQueueItem << " | " << productHeader->pCurrent << " \\ " << productHeader->QueueSize << endl;
//    cout << productHeader->pNextQueueItem%productHeader->QueueSize << " | " << productHeader->pCurrent%productHeader->QueueSize << " \\ " << productHeader->QueueSize << endl;

    // Note :: We use the WNOHANG to call waitpid without blocking
    // If it returns 0, it does not have a PID waiting
    waitPID = waitpid(-1, &wstatus, WNOHANG | WUNTRACED | WCONTINUED);


    // No PIDs are in-process
    if (isKilled) {
      bComplete = true;   // We say true so that we exit out of main
      break;              // loop and free up all necessary data
    }

    // Child processed correctly
    if (WIFEXITED(wstatus) && waitPID > 0)
    {
      // Decrement our ProcessCounter
      ProcessCount--;
    
    } else if (WIFSIGNALED(wstatus) && waitPID > 0) {
        cout << waitPID << " killed by signal " << WTERMSIG(wstatus) << endl;
    } else if (WIFSTOPPED(wstatus) && waitPID > 0) {
        cout << waitPID << " stopped by signal " << WTERMSIG(wstatus) << endl;
    } else if (WIFCONTINUED(wstatus) && waitPID > 0) {
        continue;
    }

  }

  // Signal to the producers to shutdown
  cout << "Shutting down producers" << endl;
  for(int i=0; i < vecProducers.size(); i++)
  {
    kill(vecProducers[i], SIGQUIT); 
    cout << "Producer " << vecProducers[i] << " shutdown" << endl;
  }

  cout << "Shutting down consumers" << endl;
  for(int i=0; i < vecConsumers.size(); i++)
  {
    kill(vecConsumers[i], SIGQUIT); 
    cout << "Consumer " << vecConsumers[i] << " shutdown" << endl;
  }

  // Breakdown shared memory
  // Dedetach shared memory segment from process's address space
  cout << endl;
  perror("De-allocating shared memory");
  if (shmdt(shm_addr) == -1) {
      perror("main: shmdt: ");
  }

  // De-allocate the shared memory segment.
  if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
      perror("main: shmctl: ");
  }

  perror("Shared memory De-allocated");
  cout << endl;

  if(isKilled)
    return EXIT_FAILURE;

  // Success!
  return EXIT_SUCCESS;
}


int forkProcess(string strProcess)
{
        pid_t pid = fork();
        // No child made - exit with failure
        if(pid < 0)
        {
            // Signal to any child process to exit

            perror("Could not fork process");
            return EXIT_FAILURE;
        }
        // Child process here - Assign out it's work
        if(pid == 0)
        {
            // Execute child process
            execl(strProcess.c_str(), NULL);

            fflush(stdout); // Mostly for debugging -> tty wasn't flushing
            exit(EXIT_SUCCESS);    // Exit from forked process successfully
        }
        else
            return pid; // Returns the Parent PID
}


