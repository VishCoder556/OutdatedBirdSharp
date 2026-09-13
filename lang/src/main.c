/*
 * BirdSharp Compiler
 * BirdSharp is a programming language that aims to be similar to C, while having many high level features.
 *
 * Why Use BirdSharp:
 *   1. BirdSharp, as stated earlier, is similar to C, but has nearly no bloat.
 *   2. BirdSharp is extremely fast (although dynamic libraries seem to slow the compile time a lot)
 *   3. BirdSharp supports bytecode and compilation directly into Assembly that can be compiled to create executables.
 *   4. BirdSharp has many features unique to it, such as modes, that allow you to change some stuff about the compiler during runtime, instead of having to rely on compiler flags for such tasks.
 *
 * NOTE: BirdSharp is not finished yet, and hence has a ton of unclean code, and places to improve upon. Do not rely on BirdSharp for complex projects.
 *
 * How to try out BirdSharp:
 *  There are two ways:
 *      1. Executables (The main version of BirdSharp):
 *          Run "bs main.bsh" to create the .s file
 *              (This will automatically assemble it into a .o file and link it into a .out)
 *          Run "./main" to run your file!
 *
 *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <wchar.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>


// From https://github.com/tsoding/arena/

#define ARENA_IMPLEMENTATION
#include "include/arena.h"
typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} String_Builder;
Arena arena = {0};

enum BirdSharpTypes {
   TYPE_STATIC,
   TYPE_DYNAMIC
};



void error_generate(char *type, char *name){
    printf("\x1b[1;31m%s\x1b[0m: %s\n", type, name);
    exit(-1);
};

void print_error_line(const char *code, int line, int col) {
    int curr_line = 1;
    const char *start = code;
    const char *end = code;

    while (curr_line < line && *end) {
        if (*end == '\n') {
            curr_line++;
            start = end + 1;
        }
        end++;
    }

    while (*end && *end != '\n') end++;

    printf("%.*s\n", (int)(end - start), start);

    int pointer_col = col;
    if (pointer_col < 1) pointer_col = 1;
    for (int i = 1; i < pointer_col; ++i) putchar(' ');
    puts("\x1b[1;31m^\x1b[0m\n");
}


void warning_generate(char *type, char *name){
    printf("\x1b[1;35m%s\x1b[0m: %s\n", type, name);
};
void warning_generate_parser(char *type, char *name, int l, int c, char *f){
    printf("\x1b[1;37m%s:%d:%d: \x1b[1;35m%s\x1b[0m: %s\n", f, l, c, type, name);
};

char* get_argument_register(int n) {
    switch (n) {
        case 0: return "rdi";
        case 1: return "rsi";
        case 2: return "rdx";
        case 3: return "rcx";
        case 4: return "r8";
        case 5: return "r9";
        default: return "stack";
    }
}




char* find_ir() {
    static char filename[256];
    int i = 0;

    while (true) {
        snprintf(filename, sizeof(filename), "tmp_%d.ir", i);

        if (access(filename, F_OK) != 0) {
            return strdup(filename);
        }

        i++;
        
        if (i > 1000) return NULL; 
    }
}


#include "tokenizer.c"
#include "parser.c"
#include "typechecker.c"
#include "generator.c"
#include "ir.c"
#include "preprocessor.c"

char *current_process = "";


#define clockend() \
    clock_gettime(CLOCK_MONOTONIC, &__end); \
    fprintf(stdout, "[INFO] %s process took %.4f milliseconds\n", current_process, (__end.tv_sec - __start.tv_sec) * 1000.0 + (__end.tv_nsec - __start.tv_nsec) / 1e6); \

#define clockbegin(name) current_process = name; clock_gettime(CLOCK_MONOTONIC, &__start);
#define clockstart(name) clockend(); clockbegin(name);


#define VERSION "v0.0.0"
char *HELP = 
"Help for BirdSharp "VERSION"\n"
"USAGE: `bsh [input_file]`\n\n"
"-o: Specify the output file\n"
"--version: Print the current version\n"
"-h or --h: Print this menu\n";

int main(int argc, char **argv){
    struct timespec __begin;
    clock_gettime(CLOCK_MONOTONIC, &__begin);
    argv++;
    char *input_file = "";
    char *output_file = "";
    fflush(stdout); // This might look stupid, but calling a function helps align or stack to prevent overflows. You'll see this line a lot in the codebase, I just want to make sure that the stack is always aligned in the correct manner.
    while (*argv){
        if (*argv[0] == '-'){
            if(strcmp(*argv, "-o") == 0){
                argv++;
                if (*argv){
                output_file = *argv;
                }else {
                    break;
                }
            }else if(strcmp(*argv, "-h") == 0 || strcmp(*argv, "--h") == 0){
                printf("%s", HELP);
                return 0;
            }else if(strcmp(*argv, "--version") == 0){
                printf("BirdSharp %s\n", VERSION);
                return 0;
            }
        } else {
            input_file = *argv;
        };
        argv++;
    };

    if (strcmp(output_file, "") == 0){
        output_file = "main";
    };

    if (!input_file || strcmp(input_file, "") == 0){
        printf("\x1b[1;31merror\x1b[0m: No input file provided\n");
        return -1;
    };

    struct timespec __start, __end;
    arena_reset(&arena);


    clockbegin("Tokenizer");


    Tokenizer *tokenizer = tokenizer_init(input_file, 1);
    while(tokenizer_token(tokenizer) != -1){};



    clockstart("Preprocessor");
    

    preprocess(input_file, tokenizer);


    clockstart("Parsing");




    Parser *parser = parse_file(tokenizer);

    tokenizer_free_tokens(tokenizer);
    tokenizer_free(tokenizer);



    clockstart("Typechecking");


    Typechecker *typechecker = typechecker_init(parser);
    while (typechecker_eat_ast(typechecker) != -1){};

    clockstart("Transpiling");


    Generator *generator = generator_make(typechecker, ir);
    
    clockstart("Compiling");


    char cmd[256];
    snprintf(cmd, sizeof(cmd), "./irc %s %s -o %s", generator->output->filename, "-target x86_64-mac", output_file);
    system(cmd);
    // snprintf(cmd, sizeof(cmd), "cat %s | pbcopy", generator->output->filename);
    // system(cmd);

    
    generator_clean(generator);

    clockend();

    fprintf(stdout, "[INFO] Program took %.4f milliseconds\n", (__end.tv_sec - __begin.tv_sec) * 1000.0 + (__end.tv_nsec - __begin.tv_nsec) / 1e6);
    
    arena_free(&arena);
    
    return 0;
}
