/********************************************
 * main - Semaphores and Message Passing
 * This file is for the main function of the
 * application.  It simply makes sure the
 * program arguements are correct, then
 * kicks off the monitor functionality for
 * processing.
 * 
 * Brett Huffman
 * CMP SCI 4760 - Project 3
 * Due Mar 12, 2021
 * Main CPP file for project
 ********************************************/
#include <iostream>
#include <string.h>
#include <vector>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libmonitor.h"
#include <errno.h>

// Constants
const int MaxNumberOfChildren = 20;
const int MaxNumberOfSeconds = 100;

// Forward declarations
static void show_usage(std::string);

// Main - expecting arguments
int main(int argc, char* argv[])
{
    // Argument processing
    int opt;
    string strLogFile = "logfile"; // Default setting
    int nNumberOfProducers = 2; // Default setting
    int nNumberOfConsumers = 6; // Default setting
    int nNumberOfSeconds = 100; // Default setting

    // Go through each parameter entered and
    // prepare for processing
    opterr = 0;
    while ((opt = getopt(argc, argv, "ho:p:c:t:")) != -1) {
        switch (opt) {
            case 'h':
                show_usage(argv[0]);
                return EXIT_SUCCESS;
            case 'o':
                strLogFile = optarg;
                break;
            case 'p':
                nNumberOfProducers = atoi(optarg);
                break;
            case 'c':
                nNumberOfConsumers = atoi(optarg);
                break;
            case 't':
                nNumberOfSeconds = atoi(optarg);
                break;
            case '?': // Unknown arguement                
                if (isprint (optopt))
                {
                    errno = EINVAL;
                    perror("Unknown option");
                }
                else
                {
                    errno = EINVAL;
                    perror("Unknown option character");
                }
                return EXIT_FAILURE;
            default:    // An bad input parameter was entered
                // Show error because a bad option was found
                perror ("master: Error: Illegal option found");
                show_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Set the correct default values (min of both)
    nNumberOfConsumers = min(nNumberOfConsumers, MaxNumberOfChildren-nNumberOfProducers);
    nNumberOfSeconds = min(nNumberOfSeconds, MaxNumberOfSeconds);

    // Check that a data file has been passed in to process
    int index = optind;
    if(index < argc)
    {
        // Get the string to process
        string FileToProcess = argv[index];

        // Output what is going to happen
        cout << "Monitor starting: " << endl 
            << "\t" << nNumberOfProducers << " Producers" << endl
            << "\t" << nNumberOfConsumers << " Consumers" << endl
            << "\t" << nNumberOfSeconds  << " Max Seconds" << endl << endl;

        // Start the Master process, returning whatever master returns.
//        return processMaster(nNumberOfChildren, nNumberOfSeconds, FileToProcess);
    }

    // Otherwise, an error -- must pass a filename
    perror ("Error: You must enter a data file to process");
    show_usage(argv[0]);
    return EXIT_FAILURE;
}


// Handle errors in input arguments by showing usage screen
static void show_usage(std::string name)
{
    std::cerr << std::endl
              << name << " - monitor app by Brett Huffman for CMP SCI 4760" << std::endl
              << std::endl
              << "Usage:\t" << name << " [-h]" << std::endl
              << "\t" << name << " [-h] [-o logfile] [-p m] [-c n] [-t time]" << std::endl
              << "Options:" << std::endl
              << "  -o logfile Name of the file to save logs; default: logfile" << std::endl
              << "  -p m Number of producers; default: m = 2" << std::endl
              << "  -c n Number of consumers; default: n = 6" << std::endl
              << "  -t time The time in seconds after which the process will terminate, even if it has not finished. (Default: 100)"
              << std::endl << std::endl;
}