#include <stdlib.h>

#include "fliki.h"
#include "global.h"
#include "debug.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 * @modifies global variable "diff_filename" to point to the name of the file
 * containing the diffs to be used.
 */

int validargs(int argc, char **argv) {

    global_options = 0;

    if(argc < 2) {
        return -1;
    }
    else {
        if(argc == 2) {
            char firstChar = **(argv + 1);
            char secondChar = *(*(argv + 1) + 1);

            if(firstChar == '-') {
                if(secondChar == 'h' && (*(*(argv + 1) + 2) == '\0')) {
                    global_options = global_options | 0x00000001;
                    return 0;
                }
                else {
                    return -1;
                }
            }
            else {
                diff_filename = *(argv + 1);
                return 0;
            }
        }

        for(int i = 1; i < argc; i++) {
            if(i == 1 && (**(argv + i) == '-') && (*(*(argv + i) + 1) == 'h') && (*(*(argv + i) + 2) == '\0')) {
                global_options = global_options | 0x00000001;
                return 0;
            }

            if(i == argc - 1) {
                if(**(argv + i) != '-') {
                    diff_filename = *(argv + i);
                    return 0;
                }
                else {
                    global_options = 0;
                    return -1;
                }
            }

            else if((**(argv + i) == '-') && (*(*(argv + i) + 1) == 'q') && (*(*(argv + i) + 2) == '\0')) {
                global_options = global_options | 0x00000004;
                }
            else if((**(argv + i) == '-') && (*(*(argv + i) + 1) == 'n') && (*(*(argv + i) + 2) == '\0')) {
                global_options = global_options | 0x00000002;
            }
            else {
                global_options = 0;
                return -1;
            }
        }
    }
    return 0;
}