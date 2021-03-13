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


// MonitorProcess - Process to start monitor process.  It kicks of Producers and Consumers as necessary
int monitorProcess(string InputDataFile, int nNumberOfProducers, int nMaxNumberOfConsumers, int nSecondsToTerminate)
{
  // Check Input and exit if a param is bad
  if(InputDataFile.size() < 1 || nNumberOfProducers < 1 || 
    nMaxNumberOfConsumers < 1 || nSecondsToTerminate < 1)
  {
    errno = EINVAL;
    perror("LibMonitor: Unknown option");
    return EXIT_FAILURE;
  }

  // Register SIGINT handling
  signal(SIGINT, sigintHandler);
  bool isKilled = false;
  bool bComplete = false;

  // Start Time for time Analysis
  time_t secondsStart;
  secondsStart = time(NULL);

  // Start counting number of live consumers
  // It should never be > nNumberConsumers
  int nConsumerCount = 0;

  // Start Logging
  string strLog = "***********************";
  WriteLogFile(strLog);
  strLog = "Started LibMonitor Process";
  WriteLogFile(strLog);


  // Create the necessary Semaphores with the
  // productSemaphores class
  productSemaphores s(KEY_MUTEX, true, 1);
  productSemaphores n(KEY_EMPTY, true, 0);
  productSemaphores e(KEY_FULL, true, PRODUCT_QUEUE_LENGTH);

  if(!s.isInitialized() || !n.isInitialized() || !e.isInitialized())
  {
    perror("LibMonitor: Could not successfully create Semaphores");
    exit(EXIT_FAILURE);
  }
  // Setup shared memory
  // allocate a shared memory segment with size of 
  // Product Header + entire Product array
  int memSize = sizeof(ProductHeader) + sizeof(ProductItem) * PRODUCT_QUEUE_LENGTH;
  shm_id = shmget(KEY_SHMEM, memSize, IPC_CREAT | IPC_EXCL | 0660);
  if (shm_id == -1) {
      perror("LibMonitor: Error allocating shared memory");
      exit(EXIT_FAILURE);
  }

  // attach the shared memory segment to our process's address space
  shm_addr = (char*)shmat(shm_id, NULL, 0);
  if (!shm_addr) { /* operation failed. */
      perror("LibMonitor: Error attaching shared memory");
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
      productItemQueue[i].readyToProcess = false;
      productItemQueue[i].itemValue = 0.0f;
  }
  
  // Start up producers by fork/exec nNumberOfProducers
  for(int i=0; i < nNumberOfProducers; i++)
  {
    // Fork and store pid Producer Vector
    pid_t pid = forkProcess(ProducerProcess);
    if(pid > 0)
    {
      vecProducers.push_back(pid);
//      cout << "Producer " << vecProducers[i] << " started" << endl;
    }
  }
  cout << "LibMonitor: Started " << vecProducers.size() << " Producers" << endl << endl;

  // Check that we actually have some producers
  if(vecProducers.size() < 1)
  {
    errno = ECANCELED;
    perror("LibMonitor: Could not create Producers");
    isKilled = true;
  }
  
  // Keep track of waits & pids
  pid_t waitPID;
  int wstatus;

  // This is the main loop of the operation.  It will
  // Loop until timeout or interrupt exit
  while(!isKilled && !sigIntFlag && !((time(NULL)-secondsStart) > nSecondsToTerminate))
  {
    // Check for newly available products to consume
    // with new consumers
    s.Wait();


    // Check for a waiting, readyToProcess queue.  Also, make sure the number
    // of Consumers is always < nNumberOfConsumers (Max Number of Consumers)
    if(productItemQueue[productHeader->pCurrent%PRODUCT_QUEUE_LENGTH].readyToProcess &&
      vecConsumers.size() < (nMaxNumberOfConsumers+1))
    {
      // For a new consumer
      cout << "LibMonitor: Assigning " << productHeader->pCurrent%PRODUCT_QUEUE_LENGTH << " to new consumer" << endl;
      pid_t pid = forkProcess(ConsumerProcess, productHeader->pCurrent%PRODUCT_QUEUE_LENGTH);
      if(pid > 0)
      {
        // Keep track of the new consumer in consumer vector
        vecConsumers.push_back(pid);

        // Increment Current Index and wrap it around if > queue size
        productHeader->pCurrent = (++productHeader->pCurrent)%PRODUCT_QUEUE_LENGTH;
        
        // Report what happened ** Move Cursor left: \033[3D
        cout << "LibMonitor: Consumer " << pid << " started" << endl;
      }
    }

    s.Signal();
    
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

      // Find the Consumer in the ConsumerVector
      for(vector<int>::const_iterator consumerItem = vecConsumers.begin(); 
          consumerItem != vecConsumers.end(); ++consumerItem)
      {

        // Find the deleted consumer and remove from the vector
        if(*consumerItem == waitPID)
        {
          cout << "LibMonitor: Removing consumer " << *consumerItem << endl;
//          cout << vecConsumers.size() << endl;
          vecConsumers.erase(consumerItem);
//          vecConsumers.shrink_to_fit();
//          cout << vecConsumers.size() << endl;
          break;
        }
      }
    
    } else if (WIFSIGNALED(wstatus) && waitPID > 0) {
        cout << waitPID << " killed by signal " << WTERMSIG(wstatus) << endl;
    } else if (WIFSTOPPED(wstatus) && waitPID > 0) {
        cout << waitPID << " stopped by signal " << WTERMSIG(wstatus) << endl;
    } else if (WIFCONTINUED(wstatus) && waitPID > 0) {
        continue;
    }

  }

  // Signal to the producers to shutdown
  cout << "LibMonitor: Shutting down producers" << endl;
  for(int i=0; i < vecProducers.size(); i++)
  {
    kill(vecProducers[i], SIGQUIT); 
    cout << "LibMonitor: Producer " << vecProducers[i] << " signaled shutdown" << endl;
  }

  cout << "LibMonitor: Shutting down consumers" << endl;
  for(int i=0; i < vecConsumers.size(); i++)
  {
    kill(vecConsumers[i], SIGQUIT); 
    cout << "LibMonitor: Consumer " << vecConsumers[i] << " signaled shutdown" << endl;
  }

  // Check for timeout
  if(sigIntFlag)
  {
    string strLog = "LibMonitor: Ctrl-C Shutdown";
    WriteLogFile(strLog);
    cout << strLog << endl;
  }
  else
  {
    string strLog = "LibMonitor: Timeout Shutdown";
    WriteLogFile(strLog);
    cout << strLog << endl;
  }

  // Breakdown shared memory
  // Dedetach shared memory segment from process's address space
  cout << endl;
  cout << "LibMonitor: De-allocating shared memory" << endl;
  if (shmdt(shm_addr) == -1) {
      perror("LibMonitor: Error detaching shared memory");
  }

  // De-allocate the shared memory segment.
  if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
      perror("LibMonitor: Error deallocating shared memory ");
  }

  cout << "LibMonitor: Shared memory De-allocated" << endl;

  if(isKilled)
    return EXIT_FAILURE;


  strLog = "LibMonitor: : Producers + Consumers terminated, dealocated shared memory and semaphore";
  WriteLogFile(strLog);


  // Success!
  return EXIT_SUCCESS;
}


int forkProcess(string strProcess, int nArrayItem)
{
        pid_t pid = fork();
        // No child made - exit with failure
        if(pid < 0)
        {
            // Signal to any child process to exit

            perror("LibMonitor: Could not fork process");
            return EXIT_FAILURE;
        }
        // Child process here - Assign out it's work
        if(pid == 0)
        {
            // Execute child process without arguements
            if(nArrayItem < 0)
              execl(strProcess.c_str(), (char*)0);
            else
            {
              // Convert int to a c_str to send to exec
              string strArrayItem = GetStringFromInt(nArrayItem);
              execl(strProcess.c_str(), strArrayItem.c_str(), (char*)0);
            }

            fflush(stdout); // Mostly for debugging -> tty wasn't flushing
            exit(EXIT_SUCCESS);    // Exit from forked process successfully
        }
        else
            return pid; // Returns the Parent PID
}


