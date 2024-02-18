#include <stdio.h>
#include <math.h>
#include <string.h>
#include <czmq.h>

#include "../include/main.h"
#include "../include/simulator.h"

FILE *OutputFile;
zsock_t* OutputSocket;

void dumpFile(short* buffer, int length) {
    fwrite(buffer, sizeof(buffer[0]), length, OutputFile);
}

void dumpSocket(short* buffer, int length) {
    zsock_send(OutputSocket, "b", buffer, (length * sizeof(short)));
    zclock_sleep(10);
}

void showHelp() {
    printf("Usage: program_name [options]\n");
    printf("Options:\n");
    printf("  -h\t\tShow this help message\n");
    printf("  -o <file>\tSet the output file. If no file specified, output will be streamed on ZMQ TCP 5555\n");
}

int main(int argc, char *argv[]) {
    void (*filePtr)(short*, int) = &dumpFile;
    void (*socketPtr)(short*, int) = &dumpSocket;

    char *outputFilename = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            showHelp();
            return 0;
        }
        
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                outputFilename = argv[i + 1];

                // Skip the next argument as it is the file name
                i++;
            }
            
            else {
                printf("Error: -o flag requires a file name argument\n");
                return 1;
            }
        }
        
        else {
            printf("Error: Unknown option '%s'\n", argv[i]);
            return 1;
        }
    }

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