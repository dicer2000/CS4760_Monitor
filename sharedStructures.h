/********************************************
 * sharedStructures - This is a file that
 * contains all the structures shared between
 * the Producer + Consumer processes.  It
 * contains library calls, the main structure
 * containing data, and semephores.
 * 
 * Brett Huffman
 * CMP SCI 4760 - Project 3
 * Due Mar 12, 2021
 * sharedStructures.h file for project
 ********************************************/

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

// Arguement processing
extern int opterr;

// Critical Section Processing
enum state { idle, want_in, in_cs };

// Shared Memory structures
struct ProductHeader {
    int pNextQueueItem; // Pointer to next item for Producer
    int pCurrent;       // Pointer to next item for Consumer
    size_t QueueSize;   // Size of the queue
};
struct ProductItem {
    pid_t producerPIDAssigned;      // Producer PID
    pid_t consumerPIDAssigned;      // Consumer PID
    bool readyToProcess;    // Ready to Process
    bool logged;            // Logged Process
    bool complete;          // Completed Process
    float itemValue;        // The actual "Product" being returned 
                            // from the Producer - A little Easter Egg
};

key_t key = 0;  // Shared key
int shm_id; // Shared Mem ident
char* shm_addr;

// Product Semaphores
const key_t KEY_MUTEX = 0x54321;
const key_t KEY_EMPTY = 0x54322;
const key_t KEY_FULL = 0x54323;

struct shmseg {
   int cntr;
   int write_complete;
   int read_complete;
};

//enum Sem_Op { WAITING = -1, WAITING, RELEASE };

// The size of our product queue
const int PRODUCT_QUEUE_LENGTH = 20;

const char* ProducerProcess = "./producer";
const char* ConsumerProcess = "./consumer";

const char* ProductLogFile = "./Monitor.log";

/***************************************************
 * Helper Functions
 * *************************************************/
// For time formatting used throughout both programs
std::string GetTimeFormatted(const char* prePendString)
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[10];
    
    // Get time
    time (&rawtime);
    timeinfo = localtime (&rawtime);

    // Format time for HH:MM:SS
    strftime (buffer,80,"%T",timeinfo);

    std::string strReturn = prePendString;
    strReturn.append(buffer);
    return strReturn;
}

// Log file writing helper function
bool WriteLogFile(std::string strLogEntry)
{
    // Open a file to write
    std::ofstream logFile (ProductLogFile, std::ofstream::out | std::ofstream::app);
    if (logFile.is_open())
    {
        // Get the current local time
//        string 
        logFile << GetTimeFormatted("").c_str() << " " << strLogEntry << std::endl;
        logFile.close();
        return true;
    }
    else
    {
        perror("Unable to write to log file");
        return false;
    }
}