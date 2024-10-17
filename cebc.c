#include <stdio.h>
#include <stdlib.h>

enum alloc_type { // Determines block allocation in c-file
    STATIC,
    DYNAMIC
};

char* input_filename = NULL;
char* output_filename = NULL; // Filename will be set to input-file name with no extension
int blocks_amount = 30000; // As per standard Brainf*ck, limit of 30,000 blocks
enum alloc_type memory_type = STATIC; // Default to static memory

int main(int argc, char** argv){

    // Parse arguments
    for(int i = 1; i < argc; i++){

        // Flags
        if(argv[i][0] == '-'){

            // Flag function
            switch (argv[i][1]){

                case 'o':// Set name for output
                    output_filename = argv[++i];
                break;

                case 'b':// Set custom block amount
                    blocks_amount = atoi(argv[++i]);
                break;

                default:
                    printf("cebc: Error: invalid argument '%s'\n", argv[i]);
                    return 1;
                break;
            }
        }
        // Load file
        else{
            input_filename = argv[++i];
        }
    }


    return 0;
}