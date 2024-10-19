#include <stdio.h>
#include <stdlib.h>

enum alloc_type { // Determines block allocation in c-file
    STATIC,
    DYNAMIC
};

char* input_filename = NULL;
char* output_filename = "output.c"; // Filename will be set to input-file name with no extension
int block_amount = 30000; // As per standard Brainf*ck, limit of 30,000 blocks
enum alloc_type memory_type = STATIC; // Default to static memory
FILE* input_file;
FILE* output_file;

char** c_file_template_static = {

    "#include <stdio.h>\n",
    "#inlcude <stdlib.h>\n"
    "register int pointer = 0;\n",
    "static __uint8_t* blocks\n",
    "int main(){\n",
    "for(register int i = 0; i < ",
    NULL,// Add block amount
    "; i++) blocks[i] = 0;\n",
    NULL,

};

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
                    block_amount = atoi(argv[++i]);
                break;

                case 's':// Set memory type to static
                    memory_type = STATIC;
                break;

                case 'd':// Set memory type to dynamic
                    memory_type = DYNAMIC;
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

    // Open the Brainf*ck file
    input_file = fopen(input_filename, "r");

    // Check file status
    if(!input_file){
        printf("cebc: Error: input file '%s' could not be found", input_filename);
        return 1;
    }

    // Open the c-output file
    output_file = fopen(output_filename, "w");

    int count = 0;
    char* strptr = c_file_template_static[count];

    while(strptr){
        fprintf(output_file, "%s", strptr);
        count++;
        strptr = c_file_template_static[count];
    }

    fprintf(output_file, "%d", block_amount);
    count++;
    strptr = c_file_template_static[count];

    while(strptr){
        fprintf(output_file, "%s", strptr);
        count++;
        strptr = c_file_template_static[count];
    }

    // Read all characters in the file
    char tmp;
    while((tmp = getc(input_file))){

        switch (tmp){

            case '<':
                fprintf(output_file, "pointer--;\n");
            break;

            case '>':
                fprintf(output_file, "pointer++;\n");
            break;

            case '-':
                fprintf(output_file, "blocks[pointer]--;\n");
            break;

            case '+':
                fprintf(output_file, "blocks[pointer]++;\n");
            break;

            case '.':
                fprintf(output_file, "blocks[pointer] = getchar();\n");
            break;

            case ',':
                fprintf(output_file, "putchar(blocks[pointer]);\n");
            break;

            case '[':
                fprintf(output_file, "while(blocks[pointer]){\n");
            break;

            case ']':
                fprintf(output_file, "}\n");
            break;

        }
    }

    fprintf(output_file, "return 0;}");
    fclose(output_file);
    fclose(input_file);
    return 0;
}