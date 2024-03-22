#include <stdio.h>
#include <math.h>

#define INDEX_MULTIPLIER 12
#define TABLE_SIZE (1 << INDEX_MULTIPLIER)
#define AMPLITUDE (250.0)
#define RESOLUTION (360.0 / TABLE_SIZE)

#define WIDTH 16

enum {
    SIN,
    COS
};

void generateTable(FILE* file, int function) {
    switch (function) {
        case SIN:
            fprintf(file, "const int sinTable[%i] = {\n", TABLE_SIZE);
            break;
        
        case COS:
            fprintf(file, "const int cosTable[%i] = {\n", TABLE_SIZE);
            break;
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
        // Init variables
        int value;
        double angle;

        // Insert tab at begining of line
        if ((i % WIDTH) == 0) {
            fprintf(file, "\t");
        }

        // Generate value
        angle = (i * RESOLUTION) * M_PI / 180.0;

        switch (function) {
            case SIN:
                value = (int)(AMPLITUDE * sin(angle));
                break;
            
            case COS:
                value = (int)(AMPLITUDE * cos(angle));
                break;
        }

        // Write value
        fprintf(file, "%d, ", value);

        // Insert newline at end of line
        if (((i + 1) % WIDTH) == 0) {
            fprintf(file, "\n");
        }
    }

    fprintf(file, "};\n\n");
}

int main() {
    FILE* file;

    file = fopen("trig-tables.h", "w");

    if (file == NULL) {
        printf("ERROR OPENING FILE\n");
        return 1;
    }

    fprintf(file, "#ifndef TRIG_TABLES_H\n");
    fprintf(file, "#define TRIG_TABLES_H\n\n");

    generateTable(file, SIN);
    generateTable(file, COS);

    fprintf(file, "#endif\n");

    fclose(file);
    printf("TRIG TABLES GENERATED SUCCESSFULLY!\n");

    return 0;
}