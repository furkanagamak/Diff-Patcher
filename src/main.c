#include <stdio.h>
#include <stdlib.h>

#include "fliki.h"
#include "global.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);

    FILE* filePointer = fopen(diff_filename, "r");

    if(filePointer == NULL) {
        fprintf(stderr, "The file could not be opened.\n");
        return EXIT_FAILURE;
    }

    int patchResult = 0;

    patchResult = patch(stdin, stdout, filePointer);
    fclose(filePointer);

    if(patchResult) {
        return EXIT_FAILURE;
    }

    else {
        return EXIT_SUCCESS;
    }
}