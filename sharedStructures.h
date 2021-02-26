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

// Arguement processing
extern int opterr;

// Semiphore union
union semun {
    int                 val;
    struct semid_ds *   buf;
    unsigned short *    array;
#if defined(__linux__)
    struct seminfo *    __buf;
#endif
};

// Critical Section Processing
enum state { idle, want_in, in_cs };

// Shared Memory structure
struct AddItem {
    bool readyToProcess;    // Ready to Process
    pid_t pidAssigned;      // In-Process
    bool complete;          // Completed Process
    int nodeDepth;          // Depth that this node is processing
    int itemValue;          // The actual value
    state itemState;        // The Critical Secion Flag
};

key_t key = 0;  // Shared key
int shm_id; // Shared Mem ident
char* shm_addr;

const char* HostProcess = "./monitor";
const char* ChildProcess = "./consumer";
