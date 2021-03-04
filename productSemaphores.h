/********************************************
 * productSemaphores - Semaphores Helper Class
 * Code from CMPSCI 4760
 * 
 * This is a helper class for this project use
 * of System V Semaphores.
 * 
 * Brett Huffman
 * CMP SCI 4760 - Project 3
 * Due Mar 12, 2021
 ********************************************/
#pragma once

#include <sys/sem.h>


class productSemaphores
{
    private:
    
        bool _bCreator;
        int _semid;
        bool _isInitialized;
        struct sembuf structSemaBuf;

    public:

    productSemaphores(key_t, bool, int = 1);
    ~productSemaphores();

    // Check if properly setup
    bool isInitialized() { return _isInitialized; };

    // Semaphore Wait
    void Wait();

    // Semaphore Signal
    void Signal();    

};