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
#include <errno.h>
#include "libmonitor.h"

// Constants
// Only 20 total: Producers + Consumers + 1 Monitor
// Monitor always take 1, so we set to 19
const int MaxNumberOfChildren = 19;
const int MaxNumberOfSeconds = 100;

// Forward declarations
static void show_usage(std::string);

using namespace std;

// Main - expecting arguments
int main(int argc, char* argv[])
{
    // This main area will only handle the processing
    // of the incoming arguments. After that, all processing
    // will happen within the llibmonitor library functions.

    string strLog =  "Monitor app by Brett Huffman for CMP SCI 4760";
    cout << endl << strLog << endl << endl;

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

    if(nNumberOfConsumers < nNumberOfProducers)
    {
        perror ("master: Error: You must have more Consumers than Producers");
        show_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Output what is going to happen
    cout << "Monitor starting: " << endl 
        << "\t" << nNumberOfProducers << " Producers" << endl
        << "\t" << nNumberOfConsumers << " Consumers" << endl
        << "\t" << nNumberOfSeconds  << " Max Seconds" << endl << endl;

    // Start the monitor process, returning whatever monitor returns.
    return monitorProcess(strLogFile, nNumberOfProducers, nNumberOfConsumers, nNumberOfSeconds);

}


// Handle errors in input arguments by showing usage screen
static void show_usage(std::string name)
{
    std::cerr << std::endl
              << name << " - Monitor app by Brett Huffman for CMP SCI 4760" << std::endl
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