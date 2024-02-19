#include <stdio.h>
#include <math.h>
#include <string.h>
#include <czmq.h>

#include "../libs/rtklib-2.4.3/src/rtklib.h"

#include "../include/main.h"
#include "../include/simulator.h"

FILE *OutputFile;
zsock_t* OutputSocket;

nav_t* Ephemerides;

void dumpFile(short* buffer, int length) {
    fwrite(buffer, sizeof(buffer[0]), length, OutputFile);
}

void dumpSocket(short* buffer, int length) {
    zsock_send(OutputSocket, "b", buffer, (length * sizeof(short)));
    zclock_sleep(10);
}

void loadEphemerides(char* ephemeridesFilename) {
    readnav(ephemeridesFilename, Ephemerides);
    uniqnav(Ephemerides);
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

    loadEphemerides(ephemeridesFilename);

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