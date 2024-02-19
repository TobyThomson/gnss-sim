#include <stdio.h>
#include <math.h>
#include <string.h>
#include <czmq.h>

#include "../libs/rtklib-2.4.3/src/rtklib.h"

#include "../include/main.h"
#include "../include/simulator.h"

FILE *OutputFile;
zsock_t* OutputSocket;

nav_t Ephemerides;

void dumpFile(short* buffer, int length) {
    fwrite(buffer, sizeof(buffer[0]), length, OutputFile);
}

void dumpSocket(short* buffer, int length) {
    zsock_send(OutputSocket, "b", buffer, (length * sizeof(short)));
    zclock_sleep(10);
}

static int compareEphemerides(const void *p1, const void *p2) {
    eph_t *q1 = (eph_t *)p1;
    eph_t *q2 = (eph_t *)p2;
    
    // Compare based on satellite ID and time of ephemeris
    if (q1->sat != q2->sat) {
        return q1->sat - q2->sat;
    }

    // Sort in descending order of time
    else {
        return q2->toe.time - q1->toe.time;
    }
}

int loadGPSEphemerides(char* ephemeridesFilename) {
    // Only interested in GPS ephemerides for now
    char* opt = "-SYS=G";

    // Open ephemerides file
    if (!readrnx(ephemeridesFilename, NULL, opt, NULL, &Ephemerides, NULL)) {
        printf("Error: Could not open ephemerides file\n");
        return -1;
    }

    // Sort the array based on satellite ID and time of ephemeris
    qsort(Ephemerides.eph, Ephemerides.n, sizeof(eph_t), compareEphemerides);

    // Declare iterators
    int i, j;
    
    // Iterate through the array to keep only the most recent ephemeris for each satellite ID
    for (i = 0, j = 1; j < Ephemerides.n; j++) {
        if (Ephemerides.eph[j].sat != Ephemerides.eph[i].sat) {
            // Move to the next satellite ID
            i++;

            // Keep the most recent ephemeris for this satellite ID
            Ephemerides.eph[i] = Ephemerides.eph[j];
        }
    }

    // Update the number of ephemeris entries after removing duplicates
    Ephemerides.n = i + 1;

    // Display result!
    printf("GPS EPHEMERIDES LOADED: %i\n", Ephemerides.n);

    return 0;
}

void showHelp() {
    printf("Usage: program_name [options]\n");
    printf("Options:\n");
    printf("  -h\t\tShow this help message\n");
    printf("  -e <file>\tSet the ephemerides file. Required!\n");
    printf("  -o <file>\tSet the output file. If no file specified, output will be streamed on ZMQ TCP 5555\n");
}

int main(int argc, char *argv[]) {
    void (*filePtr)(short*, int) = &dumpFile;
    void (*socketPtr)(short*, int) = &dumpSocket;

    char *ephemeridesFilename = NULL;
    char *outputFilename = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            showHelp();
            return 0;
        }

        else if (strcmp(argv[i], "-e") == 0) {
            if (i + 1 < argc) {
                ephemeridesFilename = argv[i + 1];

                // Skip the next argument as it is the filename
                i++;
            }
            
            else {
                printf("Error: -e flag requires a filename argument\n");
                return 1;
            }
        }
        
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                outputFilename = argv[i + 1];

                // Skip the next argument as it is the filename
                i++;
            }
            
            else {
                printf("Error: -o flag requires a filename argument\n");
                return 1;
            }
        }
        
        else {
            printf("Error: Unknown option '%s'\n", argv[i]);
            return 1;
        }
    }

    if (!ephemeridesFilename) {
        printf("Error: ephemerides must be provided. See help\n");
        return 0;
    }

    loadGPSEphemerides(ephemeridesFilename);

    // Enter file mode if output file specified
    if (outputFilename) {
        printf("WRITING DATA TO FILE...\n");
        OutputFile = fopen(outputFilename, "wb");
        simulate(filePtr);
        fclose(OutputFile);
    }

    // Otherwise, enter streaming mode
    else {
        printf("STREAMING DATA...\n");
        OutputSocket = zsock_new_pub("tcp://127.0.0.1:5555");

        // Sleep to give subscriber time to connect
        sleep(1);

        simulate(socketPtr);

        // Sleep to give subscriber time to collect
        sleep(1);

        zsock_destroy(&OutputSocket);
    }

    return 0;
}