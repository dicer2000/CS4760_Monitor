
#include <sys/sem.h>
#include <iostream>
#include "productSemaphores.h"

using namespace std;

productSemaphores::productSemaphores(key_t key, bool Create)
{
    // If a valid key
    if(key > 0)
    {
        // If Creating a new Key
        if(Create)
        {
            _semid = semget(key, 1, SEM_R | SEM_A | IPC_EXCL | IPC_CREAT);
            // If successful, set it's value to 1
            if (_semid > 0)
            {
                semctl(_semid, 0, SETVAL, 1);
                // Write success to log file

                // Set as the creator of the Sem
                _bCreator = true;
                // Set as properly initialized
                _isInitialized = true;
            }
        }
        else
        {
            // Get an already created Semaphore
            _semid = semget(key, 1, SEM_R | SEM_A );
            _bCreator = false;
            if (_semid > 0)
            {
                // Set as properly initialized
                _isInitialized = true;
            }
        }
    }
}

productSemaphores::~productSemaphores()
{
    if(_bCreator && _isInitialized)
    {
        semctl(_semid, 0, IPC_RMID);

        // Log as released
    }
}

void productSemaphores::Wait()
{
    structSemaBuf.sem_num = 0;
    structSemaBuf.sem_op = -1;
    structSemaBuf.sem_flg = 0;
    semop(_semid, &structSemaBuf, 1);
	cout << "wait: " << _semid << endl;

}

// Semaphore Signal
void productSemaphores::Signal() 
{
    structSemaBuf.sem_num = 0;
    structSemaBuf.sem_op = 1;
    structSemaBuf.sem_flg = 0;
    semop(_semid, &structSemaBuf, 1);
	cout << "signal: " << _semid << endl;
}

