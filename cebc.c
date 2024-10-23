#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CEBC_VERSION_NUMBER "1.0.1"

#define DEFAULT_C_FILE_OUTPUT_FILENAME "cebc_out.c"
#define DEFAULT_BINARY_OUTPUT_NAME "cebc_out"

enum alloc_type { // Determines block allocation in c-file
    STATIC,
    DYNAMIC
};

enum process_type{ // Determines if the just the c file should be built or full compile
    BUILD_C_FILE,
    COMPILE
};

static char*  c_file_template_static[] = {

    "#include <stdio.h>\n",
    "#include <stdlib.h>\n",
    "int main(){\n",
    "register __uint32_t p=0;\n",
    "__uint8_t b[",
    NULL,
    "];\n",
    "for(register int i=0;i<",
    NULL,
    ";i++)b[i]=0;\n",
    NULL,
};

static char*  c_file_template_dynamic[] = {

    "#include <stdio.h>\n",
    "#include <stdlib.h>\n",
    "int main(){\n",
    "register __uint32_t p=0;\n",
    "__uint8_t* b;",
    "b=malloc(",
    NULL,
    ");\n",
    "for(register int i=0;i<",
    NULL,
    ";i++)b[i]=0;\n",
    NULL,
};

void write_file_opening(enum alloc_type mem_type, FILE* output_file, __uint32_t block_amount);

int write_file_contents(const char* input_filename, FILE* output_file, const char* output_filename);

void write_file_ending(enum alloc_type mem_type, FILE* output_file);

void print_options_and_exit();

int main(int argc, char** argv){

    enum alloc_type memory_type = STATIC; // Default to static memory
    enum process_type process = COMPILE; // Default to full process
    char** input_filenames = NULL; // All files to be compiled
    __uint32_t input_filenames_allocated_size = 0; // Space allocated in input_filenames
    __uint32_t input_filenames_count = 0; // The number of files to be compiled
    char* output_name = NULL; // Default output file
    __uint32_t block_amount = 30000; // As per standard Brainf*ck, defautlt limit of 30,000 blocks


    // Parse arguments
    for(int i = 1; i < argc; i++){

        // Argument indicated as option flag
        if(argv[i][0] == '-'){

            if(!strcmp(argv[i] + 1, "h")){
                print_options_and_exit();
            }// List flags

            else if(!strcmp(argv[i] + 1, "o")){
                output_name = argv[++i];
            }// Specify custom output file name

            else if(!strcmp(argv[i] + 1, "b")){
                block_amount = atoi(argv[++i]);

                if(!block_amount){
                    printf("cebc: Error: invalid block amount '%s'\n", argv[i]);
                }
            }// Set custom block amount

            else if(!strcmp(argv[i] + 1, "s")){
                memory_type = STATIC;
            }// Set memory type to static

            else if(!strcmp(argv[i] + 1, "d")){
                memory_type = DYNAMIC;
            }// Set memory type to dynamic

            else if(!strcmp(argv[i] + 1, "c")){
                process = BUILD_C_FILE;
            }// Set process to partial

            else{
                printf("cebc: Error: invalid option flag '%s'\n", argv[i]);
                return 1;
            }// Invalid option flag
        }

        // Argument is treated as an input filename
        else{

            if(!input_filenames_count){// First filename
                input_filenames = malloc(sizeof(char*));
                
                if(!input_filenames){
                    printf("cebc: Error: malloc failure while loading filename(s)\n");
                    return 1;
                }// Error reallocating

                input_filenames[input_filenames_count] = argv[i];
                input_filenames_allocated_size = 1;
            }

            else{// Additional filenames
                if(input_filenames_count + 1 > input_filenames_allocated_size){// Needs to be reallocated
                    input_filenames_allocated_size *= 2;
                    char** tmp_ptr = realloc(input_filenames, input_filenames_allocated_size * sizeof(char*));

                    if(!tmp_ptr){
                        printf("cebc: Error: realloc failure while loading filename(s)\n");
                        return 1;
                    }// Error reallocating

                    input_filenames = tmp_ptr;
                }

                input_filenames[input_filenames_count] = argv[i];
            }

            input_filenames_count++;
        }
    }

    if(!input_filenames_count){
        printf("cebc: Error: no input file(s)\n");
        return 1;
    }// No files specified

    char* c_filename = DEFAULT_C_FILE_OUTPUT_FILENAME;

    if(process == BUILD_C_FILE){
        if(output_name){
            c_filename = output_name;
        }
    }// Use specified name for c_file output
    else if(process == COMPILE){
        if(!output_name){
            output_name = DEFAULT_BINARY_OUTPUT_NAME;
        }
    }// Use default binary name if not specified

    FILE* output_file;
    output_file = fopen(c_filename, "w");

    if(!output_file){
        printf("cebc: Error: failure creating intermediary c-file (bad output filename?)\n");
        return 1;
    }// Can't create file / bad filename?

    write_file_opening(memory_type, output_file, block_amount);
    
    for(int i = 0; i < input_filenames_count; i++){// Write all files to c
        if(!write_file_contents(input_filenames[i], output_file, c_filename)){
            printf("cebc: Error: unequal conditional brackets '[' ']' (no matching) \n");
            fclose(output_file);
            remove(c_filename);
            return 1;
        }// Missing brackets / not matching (remove c-file)
    }

    write_file_ending(memory_type, output_file);
    fclose(output_file);

    if(process == COMPILE){
        char compile_command[256];
        snprintf(compile_command, 248, "gcc -o %s %s", output_name, c_filename);
        
        system(compile_command);
        remove(c_filename);
    }

    return 0;
}

void write_file_opening(enum alloc_type mem_type, FILE* output_file, __uint32_t block_amount){

    char** c_file_template = NULL;

    if(mem_type == STATIC){
        c_file_template = c_file_template_static;
    }// Static allocation

    if(mem_type == DYNAMIC){
        c_file_template = c_file_template_dynamic;
    }// Dynamic allocation

    for(int i = 0, null_count = 0; null_count < 3; i++){

        if(!c_file_template[i]){
            null_count++;
            if(null_count < 3){
                fprintf(output_file, "%d", block_amount);
            }// Don't print at last null
        }// Null encountered (insert block amount)
        else{
            fprintf(output_file, "%s", c_file_template[i]);
        }// Normal template line
    }
}

int write_file_contents(const char* input_filename, FILE* output_file, const char* output_filename){

    FILE* input_file;
    input_file = fopen(input_filename, "r");

    if(!input_file){
        printf("cebc: Error: input file '%s' not found", input_filename);
        fclose(output_file);
        remove(output_filename);
        exit(1);
    }// Unable to locate file (remove c-file)

    register __uint32_t left_bracket_count = 0;
    register __uint32_t right_bracket_count = 0;

    // Read all characters in the file
    register char tmp = fgetc(input_file);
    while(tmp != EOF){

        switch (tmp){

            case '<':
                fprintf(output_file, "p--;\n");
            break;

            case '>':
                fprintf(output_file, "p++;\n");
            break;

            case '-':
                fprintf(output_file, "b[p]--;\n");
            break;

            case '+':
                fprintf(output_file, "b[p]++;\n");
            break;

            case ',':
                fprintf(output_file, "b[p] = getchar();\n");
            break;

            case '.':
                fprintf(output_file, "putchar(b[p]);\n");
            break;

            case '[':
                fprintf(output_file, "while(b[p]){\n");
                left_bracket_count++;
            break;

            case ']':
                fprintf(output_file, "}\n");
                right_bracket_count++;
            break;

            default:
            break;
        }
        tmp = fgetc(input_file);
    }

    fclose(input_file);

    return (left_bracket_count == right_bracket_count) ? 1 : 0;
}

void write_file_ending(enum alloc_type mem_type, FILE* output_file){

    if(mem_type == STATIC){
        fprintf(output_file, "return 0;\n}");
    }// Static allocation

    if(mem_type == DYNAMIC){
        fprintf(output_file, "free(b);\nreturn 0;\n}");
    }// Dynamic allocation
}

void print_options_and_exit(){

    printf("Cerebral-Compiler 'cebc': Version - %s\n", CEBC_VERSION_NUMBER);
    puts("Options: --------------------------------------------------------------------");
    puts(" '-h'  Print all options for cebc");
    puts(" '-o'  Specify the name of the output binary or c-file");
    puts(" '-b'  Specify the number of blocks allocated from memory (default is 30,000)");
    puts(" '-s'  Generate binary / c-file with static memory allocation");
    puts(" '-d'  Generate binary / c-file with dynamic memory allocation");
    puts(" '-c'  Generate c-file only, don't compile to executible");

    exit(0);
}