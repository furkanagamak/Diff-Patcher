#include <stdlib.h>
#include <stdio.h>

#include "fliki.h"
#include "global.h"
#include "debug.h"

static int additionsFlag = 0;
static int deletionsFlag = 0;
static int firstChangeReadFlag = 0;
static int getcErrorFlag = 0;
static int incrementAddBuffer = 0;
static int incrementDeleteBuffer = 0;
static int EOSCount = 0;
static int addCharCount = 0;
static int deleteCharCount = 0;
static int numOfAdditionLines = 0;
static int numOfDeletionLines = 0;

static int charTracker  = '\n';
static int lineCounter = 0;

static int addOverflowFlag = 0;
static int deleteOverflowFlag = 0;

static int changeFlag = 0;
static int addSetCharFlag = 0;
static int deleteSetCharFlag = 0;
static int numOfCharsStored = 0;
/**
 * @brief Get the header of the next hunk in a diff file.
 * @details This function advances to the beginning of the next hunk
 * in a diff file, reads and parses the header line of the hunk,
 * and initializes a HUNK structure with the result.
 *
 * @param hp  Pointer to a HUNK structure provided by the caller.
 * Information about the next hunk will be stored in this structure.
 * @param in  Input stream from which hunks are being read.
 * @return 0  if the next hunk was successfully located and parsed,
 * EOF if end-of-file was encountered or there was an error reading
 * from the input stream, or ERR if the data in the input stream
 * could not be properly interpreted as a hunk.
 */

int hunk_next(HUNK *hp, FILE *in) {
    numOfCharsStored = 0;
    addSetCharFlag = 0;
    deleteSetCharFlag = 0;
    changeFlag = 0;
    additionsFlag = 0;
    deletionsFlag = 0;
    firstChangeReadFlag = 0;
    getcErrorFlag = 0;
    incrementAddBuffer = 0;
    incrementDeleteBuffer = 0;
    EOSCount = 0;
    addCharCount = 0;
    deleteCharCount = 0;
    numOfAdditionLines = 0;
    numOfDeletionLines = 0;

    addOverflowFlag = 0;
    deleteOverflowFlag = 0;

    hp->old_end = 0;
    hp->old_start = 0;
    hp->new_start = 0;
    hp->new_end = 0;

    for(int i = 0; i < HUNK_MAX; i++) {
        *(hunk_deletions_buffer + i) = 0;
        *(hunk_additions_buffer + i) = 0;
    }

    int c = 0;
    int num = 0;

    if(charTracker == EOF) {
        return EOF;
    }

    if(charTracker != '\n') {
        while(c != '\n') {
            c = fgetc(in);
            charTracker = c;

            if(c == EOF) {
                return ERR;
            }
        }
        lineCounter++;
    }

    c = fgetc(in);
    charTracker = c;

    if(c == EOF) {
        return EOF;
    }

    while(!(c >= '0' && c <= '9')) {
        if(c == '>') {
            if(hp->type != HUNK_APPEND_TYPE && hp->type != HUNK_CHANGE_TYPE) {
                return ERR;
            }

        c = fgetc(in);
        charTracker = c;

        if(c == ' ') {
            while(c != '\n') {
                c = fgetc(in);
                charTracker = c;

                if(c == EOF) {
                    return ERR;
                }
            }
            lineCounter++;
        }

        else {
            return ERR;
        }

        c = fgetc(in);
        charTracker = c;
    }

    else if(c == '<') {
        if(hp->type != HUNK_DELETE_TYPE && hp->type != HUNK_CHANGE_TYPE) {
                return ERR;
        }


        c = fgetc(in);
        charTracker = c;

        if(c == ' ') {
            while(c != '\n') {
                c = fgetc(in);
                charTracker = c;

                if(c == EOF) {
                    return ERR;
                }
            }
            lineCounter++;
        }
        else {
            return ERR;
        }

        c = fgetc(in);
        charTracker = c;
    }

    else if(c == '-') {
        if(hp->type != HUNK_CHANGE_TYPE) {
            return ERR;
        }

        if(changeFlag == 1) {
            return ERR;
        }

        c = fgetc(in);
        charTracker = c;

        if(c == '-') {
            c = fgetc(in);
            charTracker = c;

            if(c == '-') {
                c = fgetc(in);
                charTracker = c;

                if(c != '\n') {
                    return ERR;
                }
                changeFlag = 1;
                lineCounter++;
            }
            else {
                return ERR;
            }
        }
        else {
            return ERR;
        }

        c = fgetc(in);
        charTracker = c;
    }

    else if(c == EOF) {
        return EOF;
    }

    else {
        return ERR;
    }
}


    if(c >= '0' && c <= '9') {
            num = c - '0';

            c = fgetc(in);
            charTracker = c;

            while(c >= '0' && c <= '9') {
                num = num * 10 + c - '0';
                c = fgetc(in);
                charTracker = c;
            }


            hp->old_start = num;
            hp->old_end = num;

            if(c == ',') {
                c = fgetc(in);
                charTracker = c;

                if(c >= '0' && c <= '9') {
                    num = c - '0';

                    c = fgetc(in);
                    charTracker = c;

                    while(c >= '0' && c <= '9') {
                        num = num * 10 + c - '0';
                        c = fgetc(in);
                        charTracker = c;
                    }

                    hp->old_end = num;

                    if(hp->old_end < hp->old_start) {
                        hp->type = 0;
                        return ERR;
                    }

                    if(c == 'c') {
                        hp->type = HUNK_CHANGE_TYPE;
                        c = fgetc(in);
                        charTracker = c;

                        if(c >= '0' && c <= '9') {
                            num = c - '0';

                                                c = fgetc(in);
                    charTracker = c;

                    while(c >= '0' && c <= '9') {
                        num = num * 10 + c - '0';
                        c = fgetc(in);
                        charTracker = c;
                    }

                            hp->new_start = num;
                            hp->new_end = num;

                            if(c == '\n') {
                                lineCounter++;
                                int temp = hp->serial + 1;
                                hp->serial = temp;

                                if(hp->old_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_DELETE_TYPE)) {
                                    return ERR;
                                }

                                if(hp->new_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_APPEND_TYPE)) {
                                    return ERR;
                                }

                                return 0;
                            }

                            else if(c == ',') {
                                c = fgetc(in);
                                charTracker = c;
                                if(c >= '0' && c <= '9') {
                                    num = c - '0';

                                    c = fgetc(in);
                                    charTracker = c;

                                    while(c >= '0' && c <= '9') {
                                        num = num * 10 + c - '0';
                                        c = fgetc(in);
                                        charTracker = c;
                                    }

                                    hp->new_end = num;

                                    if(hp->new_end < hp->new_start) {
                                        hp->type = 0;
                                        return ERR;
                                    }

                                    if(c == '\n') {
                                        lineCounter++;
                                        int temp = hp->serial + 1;
                                        hp->serial = temp;

                                        if(hp->old_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_DELETE_TYPE)) {
                                            return ERR;
                                        }

                                        if(hp->new_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_APPEND_TYPE)) {
                                            return ERR;
                                        }

                                        return 0;
                                    }
                                    else {
                                        hp->type = 0;
                                        return ERR;
                                    }
                                }
                            }
                            else {
                                hp->type = 0;
                                return ERR;
                            }
                        }

                        else {
                            hp->type = 0;
                            return ERR;
                        }
                    }

                    else if(c == 'd') {
                        hp->type = HUNK_DELETE_TYPE;
                        c = fgetc(in);
                        charTracker = c;

                        if(c >= '0' && c <= '9') {
                            num = c - '0';
                    c = fgetc(in);
                    charTracker = c;

                    while(c >= '0' && c <= '9') {
                        num = num * 10 + c - '0';
                        c = fgetc(in);
                        charTracker = c;
                    }

                            hp->new_start = num;
                            hp->new_end = num;

                            if(c == '\n') {
                                lineCounter++;
                                int temp = hp->serial + 1;
                                hp->serial = temp;

                                if(hp->old_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_DELETE_TYPE)) {
                                    return ERR;
                                }

                                if(hp->new_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_APPEND_TYPE)) {
                                    return ERR;
                                }

                                return 0;
                            }
                            else {
                                hp->type = 0;
                                return ERR;
                            }
                        }

                        else {
                            hp->type = 0;
                            return ERR;
                        }
                    }

                    else {
                        hp->type = 0;
                        return ERR;
                    }
                }

                else {
                    hp->type = 0;
                    return ERR;
                }
            }

            else if(c == 'a') {
                hp->type = HUNK_APPEND_TYPE;
                c = fgetc(in);
                charTracker = c;
                if(c >= '0' && c <= '9') {
                    num = c - '0';
                    c = fgetc(in);
                    charTracker = c;

                    while(c >= '0' && c <= '9') {
                        num = num * 10 + c - '0';
                        c = fgetc(in);
                        charTracker = c;
                    }

                    hp->new_start = num;
                    hp->new_end = num;

                    if(c == '\n') {
                        lineCounter++;
                        int temp = hp->serial + 1;
                        hp->serial = temp;

                        if(hp->old_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_DELETE_TYPE)) {
                                return ERR;
                        }

                        if(hp->new_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_APPEND_TYPE)) {
                                return ERR;
                        }

                        return 0;
                    }

                    else if(c == ',') {
                        c = fgetc(in);
                        charTracker = c;

                        if(c >= '0' && c <= '9') {
                            num = c - '0';
                            c = fgetc(in);
                            charTracker = c;

                            while(c >= '0' && c <= '9') {
                                num = num * 10 + c - '0';
                                c = fgetc(in);
                                charTracker = c;
                            }

                            hp->new_end = num;

                            if(hp->new_end < hp->new_start) {
                                hp->type = 0;
                                return ERR;
                            }

                            if(c == '\n') {
                                lineCounter++;
                                int temp = hp->serial + 1;
                                hp->serial = temp;

                                if(hp->old_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_DELETE_TYPE)) {
                                    return ERR;
                                }

                                if(hp->new_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_APPEND_TYPE)) {
                                    return ERR;
                                }

                                return 0;
                            }

                            else {
                                hp->type = 0;
                                return ERR;
                            }
                        }

                        else {
                            hp->type = 0;
                            return ERR;
                        }

            }
        }
    }

            else if(c == 'd') {
                hp->type = HUNK_DELETE_TYPE;
                c = fgetc(in);
                charTracker = c;
                if(c >= '0' && c <= '9') {
                            num = c - '0';
                            c = fgetc(in);
                            charTracker = c;

                            while(c >= '0' && c <= '9') {
                                num = num * 10 + c - '0';
                                c = fgetc(in);
                                charTracker = c;
                            }

                    hp->new_start = num;
                    hp->new_end = num;

                    if(c == '\n') {
                        lineCounter++;
                        int temp = hp->serial + 1;
                        hp->serial = temp;

                        if(hp->old_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_DELETE_TYPE)) {
                            return ERR;
                        }

                        if(hp->new_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_APPEND_TYPE)) {
                            return ERR;
                        }

                        return 0;
                    }
                    else {
                        hp->type = 0;
                        return ERR;
                    }
                }
                else {
                    hp->type = 0;
                    return ERR;
                }
            }

            else if(c == 'c') {
                hp->type = HUNK_CHANGE_TYPE;
                c = fgetc(in);
                charTracker = c;

                if(c >= '0' && c <= '9') {
                            num = c - '0';
                            c = fgetc(in);
                            charTracker = c;

                            while(c >= '0' && c <= '9') {
                                num = num * 10 + c - '0';
                                c = fgetc(in);
                                charTracker = c;
                            }

                    hp->new_start = num;
                    hp->new_end = num;

                    if(c == '\n') {
                        lineCounter++;
                        int temp = hp->serial + 1;
                        hp->serial = temp;

                        if(hp->old_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_DELETE_TYPE)) {
                            return ERR;
                        }

                        if(hp->new_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_APPEND_TYPE)) {
                            return ERR;
                        }

                        return 0;
                    }

                    else if(c == ',') {
                        c = fgetc(in);
                        charTracker = c;
                        if(c >= '0' && c <= '9') {
                            num = c - '0';
                            c = fgetc(in);
                            charTracker = c;

                            while(c >= '0' && c <= '9') {
                                num = num * 10 + c - '0';
                                c = fgetc(in);
                                charTracker = c;
                            }

                            hp->new_end = num;

                            if(hp->new_end < hp->new_start) {
                                hp->type = 0;
                                return ERR;
                            }

                            if(c == '\n') {
                                lineCounter++;
                                int temp = hp->serial + 1;
                                hp->serial = temp;

                                if(hp->old_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_DELETE_TYPE)) {
                                    return ERR;
                                }

                                if(hp->new_start == 0 && (hp->type == HUNK_CHANGE_TYPE || hp->type == HUNK_APPEND_TYPE)) {
                                    return ERR;
                                }

                                return 0;
                            }

                            else {
                                hp->type = 0;
                                return ERR;
                            }
                        }
                        else {
                            hp->type = 0;
                            return ERR;
                        }
                    }

                    else {
                        hp->type = 0;
                        return ERR;
                    }
                }
                else {
                    hp->type = 0;
                    return ERR;
                }
            }

            else {
                hp->type = 0;
                return ERR;
            }

        }
        else {
            hp->type = 0;
            return ERR;
        }
        hp->type = 0;
        return ERR;
    }

/**
 * @brief  Get the next character from the data portion of the hunk.
 * @details  This function gets the next character from the data
 * portion of a hunk.  The data portion of a hunk consists of one
 * or both of a deletions section and an additions section,
 * depending on the hunk type (delete, append, or change).
 * Within each section is a series of lines that begin either with
 * the character sequence "< " (for deletions), or "> " (for additions).
 * For a change hunk, which has both a deletions section and an
 * additions section, the two sections are separated by a single
 * line containing the three-character sequence "---".
 * This function returns only characters that are actually part of
 * the lines to be deleted or added; characters from the special
 * sequences "< ", "> ", and "---\n" are not returned.
 * @param hdr  Data structure containing the header of the current
 * hunk.
 *
 * @param in  The stream from which hunks are being read.
 * @return  A character that is the next character in the current
 * line of the deletions section or additions section, unless the
 * end of the section has been reached, in which case the special
 * value EOS is returned.  If the hunk is ill-formed; for example,
 * if it contains a line that is not terminated by a newline character,
 * or if end-of-file is reached in the middle of the hunk, or a hunk
 * of change type is missing an additions section, then the special
 * value ERR (error) is returned.  The value ERR will also be returned
 * if this function is called after the current hunk has been completely
 * read, unless an intervening call to hunk_next() has been made to
 * advance to the next hunk in the input.  Once ERR has been returned,
 * then further calls to this function will continue to return ERR,
 * until a successful call to call to hunk_next() has successfully
 * advanced to the next hunk.
 */

int hunk_getc(HUNK *hp, FILE *in) {
    int c = 0;

    if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
        addOverflowFlag = 1;
    }

    if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
        deleteOverflowFlag = 1;
    }

    if(deleteOverflowFlag && addSetCharFlag != 1) {
        addSetCharFlag = 1;
        *(hunk_deletions_buffer + incrementDeleteBuffer - deleteCharCount) = (deleteCharCount) % 256;
        *(hunk_deletions_buffer + incrementDeleteBuffer - deleteCharCount + 1) = (deleteCharCount) / 256;

    }

    if(addOverflowFlag && deleteSetCharFlag != 1) {
        deleteSetCharFlag = 1;
        *(hunk_additions_buffer + incrementAddBuffer - addCharCount) = (addCharCount) % 256;
        *(hunk_additions_buffer + incrementAddBuffer - addCharCount + 1) = (addCharCount) / 256;
    }

    if(getcErrorFlag == 1) {
        return ERR;
    }

    if(((hp->type == HUNK_APPEND_TYPE || hp->type == HUNK_DELETE_TYPE) && EOSCount == 1) || (hp->type == HUNK_CHANGE_TYPE && EOSCount == 2)) {
        return ERR;
    }

    if(charTracker == '\n') {
        //printf("%d\n1: ", addCharCount);
        //printf("%d\n2: ", deleteCharCount);

        if(addCharCount != 0 && addOverflowFlag != 1) {
            *(hunk_additions_buffer + 2 + incrementAddBuffer) = '\n';
            *(hunk_additions_buffer + incrementAddBuffer - addCharCount) = (addCharCount + 1) % 256;
            *(hunk_additions_buffer + incrementAddBuffer - addCharCount + 1) = (addCharCount + 1) / 256;

            incrementAddBuffer++;
            incrementAddBuffer = incrementAddBuffer + 2;

            if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
        }

        else if(deleteCharCount != 0 && deleteOverflowFlag != 1) {
            *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = '\n';
            *(hunk_deletions_buffer + incrementDeleteBuffer - deleteCharCount) = (deleteCharCount + 1) % 256;
            *(hunk_deletions_buffer + incrementDeleteBuffer - deleteCharCount + 1) = (deleteCharCount + 1) / 256;

            incrementDeleteBuffer++;
            incrementDeleteBuffer = incrementDeleteBuffer + 2;

            if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
        }


        addCharCount = 0;
        deleteCharCount = 0;

        char tempChar = charTracker;
        c = fgetc(in);
        charTracker = c;

        if(hp->type == HUNK_CHANGE_TYPE) {
            if(c == '>') {
                if(firstChangeReadFlag == 1) {
                    c = fgetc(in);
                    charTracker = c;

                    if(c == ' ') {
                        additionsFlag = 1;
                        c = fgetc(in);
                        charTracker = c;

                        if(c == '\n') {

                            if(addOverflowFlag) {
                                addCharCount = 0;
                                return c;
                            }

                            *(hunk_additions_buffer + incrementAddBuffer) = 1;
                            *(hunk_additions_buffer + 2 + incrementAddBuffer) = c;
                            incrementAddBuffer++;
                            incrementAddBuffer = incrementAddBuffer + 2;
                            addCharCount = 0;

                                        if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
                            return c;
                        }

                        if(addOverflowFlag) {
                            return c;
                        }

                        *(hunk_additions_buffer + 2 + incrementAddBuffer) = c;
                        incrementAddBuffer++;
                        addCharCount++;
                                    if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
                        return c;
                    }

                    else {
                        getcErrorFlag = 1;
                        return ERR;
                    }

                }

                else {
                    getcErrorFlag = 1;
                    return ERR;
                }
            }

            else if(c == '<') {
                if(firstChangeReadFlag == 0) {
                    c = fgetc(in);
                    charTracker = c;

                    if(c == ' ') {
                        deletionsFlag = 1;
                        c = fgetc(in);
                        charTracker = c;

                        if(c == '\n') {
                            if(deleteOverflowFlag) {
                                deleteCharCount = 0;
                                return c;
                            }

                            *(hunk_deletions_buffer + incrementDeleteBuffer) = 1;

                            *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = c;
                            incrementDeleteBuffer++;
                            incrementDeleteBuffer = incrementDeleteBuffer + 2;
                            deleteCharCount = 0;

                                        if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
                            return c;
                        }

                        if(deleteOverflowFlag) {
                            return c;
                        }

                        *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = c;
                        incrementDeleteBuffer++;
                        deleteCharCount++;

                                    if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
                        return c;
                    }

                    else {
                        getcErrorFlag = 1;
                        return ERR;
                    }
                }

                else {
                    getcErrorFlag = 1;
                    return ERR;
                }
            }

            else if(c == '-') {
                c = fgetc(in);
                charTracker = c;

                if(c == '-') {
                    c = fgetc(in);
                    charTracker = c;

                    if(c == '-') {
                        c = fgetc(in);
                        charTracker = c;

                        if(c == '\n') {
                            //printf("%d\n3: ", addCharCount);
                            //printf("%d\n4: ", deleteCharCount);
        if(addCharCount != 0 && addOverflowFlag != 1) {
            *(hunk_additions_buffer + 2 + incrementAddBuffer) = '\n';
            *(hunk_additions_buffer + incrementAddBuffer - addCharCount) = (addCharCount + 1) % 256;
            *(hunk_additions_buffer + incrementAddBuffer - addCharCount + 1) = (addCharCount + 1) / 256;

            incrementAddBuffer++;
            incrementAddBuffer = incrementAddBuffer + 2;

            if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
        }

        else if(deleteCharCount != 0 && deleteOverflowFlag != 1) {
            *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = '\n';
            *(hunk_deletions_buffer + incrementDeleteBuffer - deleteCharCount) = (deleteCharCount + 1) % 256;
            *(hunk_deletions_buffer + incrementDeleteBuffer - deleteCharCount + 1) = (deleteCharCount + 1) / 256;

            incrementDeleteBuffer++;
            incrementDeleteBuffer = incrementDeleteBuffer + 2;

            if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
        }


                            addCharCount = 0;
                            deleteCharCount = 0;

                            firstChangeReadFlag = 1;
                            additionsFlag = 1;
                            deletionsFlag = 0;

                            EOSCount++;
                            return EOS;
                        }

                        else {
                            getcErrorFlag = 1;
                            return ERR;
                        }
                    }

                    else {
                        getcErrorFlag = 1;
                        return ERR;
                    }
                }

                else {
                    getcErrorFlag = 1;
                    return ERR;
                }
            }

            else if(c >= '0' && c <= '9') {
                ungetc(c, in);
                charTracker = tempChar;

                if(firstChangeReadFlag != 1) {
                    return ERR;
                }


                EOSCount++;
                return EOS;
            }

            else if(c == EOF) {

                if(firstChangeReadFlag != 1) {
                    return ERR;
                }

                EOSCount++;
                return EOS;
            }

            else {
                getcErrorFlag = 1;
                return ERR;
            }
        }

        else if(hp->type == HUNK_DELETE_TYPE) {
            if(c == '<') {
                c = fgetc(in);
                charTracker = c;

                if(c == ' ') {
                    deletionsFlag = 1;
                    c = fgetc(in);
                    charTracker = c;

                        if(c == '\n') {
                            if(deleteOverflowFlag) {
                                deleteCharCount = 0;
                                return c;
                            }

                            *(hunk_deletions_buffer + incrementDeleteBuffer) = 1;
                            
                            *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = c;
                            incrementDeleteBuffer++;
                            incrementDeleteBuffer = incrementDeleteBuffer + 2;
                            deleteCharCount = 0;

                                        if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
                            return c;
                        }

                    if(deleteOverflowFlag) {
                        return c;
                    } 

                    *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = c;
                    incrementDeleteBuffer++;
                    deleteCharCount++;
                                if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
                    return c;
                }

                else {
                    getcErrorFlag = 1;
                    return ERR;
                }
            }

            else if(c >= '0' && c <= '9') {
                ungetc(c, in);
                charTracker = tempChar;
                EOSCount++;
                return EOS;
            }

            else if(c == EOF) {
                EOSCount++;
                return EOS;
            }

            else {
                getcErrorFlag = 1;
                return ERR;
            }
        }

        else if(hp->type == HUNK_APPEND_TYPE) {
            if(c == '>') {
                c = fgetc(in);
                charTracker = c;

                if(c == ' ') {
                    additionsFlag = 1;
                    c = fgetc(in);
                    charTracker = c;

                    if(c == '\n') {
                        if(addOverflowFlag) {
                            addCharCount = 0;
                            return c;
                        }

                        *(hunk_additions_buffer + incrementAddBuffer) = 1;
                        *(hunk_additions_buffer + 2 + incrementAddBuffer) = c;
                        incrementAddBuffer++;
                        incrementAddBuffer = incrementAddBuffer + 2;
                        addCharCount = 0;
                                    if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
                        return c;
                    }

                    if(addOverflowFlag) {
                        return c;
                    }

                    *(hunk_additions_buffer + 2 + incrementAddBuffer) = c;
                    incrementAddBuffer++;
                    addCharCount++;
                                if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
                    return c;
                }

                else {
                    getcErrorFlag = 1;
                    return ERR;
                }
            }

            else if(c >= '0' && c <= '9') {
                ungetc(c, in);
                charTracker = tempChar;
                EOSCount++;
                return EOS;
            }

            else if(c == EOF) {
                EOSCount++;
                return EOS;
            }

            else {
                getcErrorFlag = 1;
                return ERR;
            }
        }

        else {
            getcErrorFlag = 1;
            return ERR;
        }
    }

    else {
        c = fgetc(in);
        charTracker = c;

        if(charTracker == '\n') {
        //printf("%d\n5: ", addCharCount);
        //printf("%d\n6: ", deleteCharCount);
        return charTracker;

        if(addCharCount != 0 && addOverflowFlag != 1) {
            *(hunk_additions_buffer + 2 + incrementAddBuffer) = '\n';
            *(hunk_additions_buffer + incrementAddBuffer - addCharCount) = (addCharCount + 1) % 256;
            *(hunk_additions_buffer + incrementAddBuffer - addCharCount + 1) = (addCharCount + 1) / 256;

            incrementAddBuffer++;
            incrementAddBuffer = incrementAddBuffer + 2;

            if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
        }

        else if(deleteCharCount != 0 && deleteOverflowFlag != 1) {
            *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = '\n';
            *(hunk_deletions_buffer + incrementDeleteBuffer - deleteCharCount) = (deleteCharCount + 1) % 256;
            *(hunk_deletions_buffer + incrementDeleteBuffer - deleteCharCount + 1) = (deleteCharCount + 1) / 256;

            incrementDeleteBuffer++;
            incrementDeleteBuffer = incrementDeleteBuffer + 2;

            if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
        }

        addCharCount = 0;
        deleteCharCount = 0;

        char tempChar = charTracker;
        c = fgetc(in);
        charTracker = c;

        if(hp->type == HUNK_CHANGE_TYPE) {
            if(c == '>') {
                if(firstChangeReadFlag == 1) {
                    c = fgetc(in);
                    charTracker = c;

                    if(c == ' ') {
                        additionsFlag = 1;
                        c = fgetc(in);
                        charTracker = c;

                        if(c == '\n') {
                            if(addOverflowFlag) {
                                addCharCount = 0;
                                return c;
                            }
                        *(hunk_additions_buffer + incrementAddBuffer) = 1;
                        *(hunk_additions_buffer + 2 + incrementAddBuffer) = c;
                        incrementAddBuffer++;
                        incrementAddBuffer = incrementAddBuffer + 2;

                                    if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
                        addCharCount = 0;
                        return c;
                    }

                    if(addOverflowFlag) {
                        return c;
                    }

                        *(hunk_additions_buffer + 2 + incrementAddBuffer) = c;
                        incrementAddBuffer++;
                        addCharCount++;

                                    if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
                        return c;
                    }

                    else {
                        getcErrorFlag = 1;
                        return ERR;
                    }

                }

                else {
                    getcErrorFlag = 1;
                    return ERR;
                }
            }

            else if(c == '<') {
                if(firstChangeReadFlag == 0) {
                    c = fgetc(in);
                    charTracker = c;

                    if(c == ' ') {
                        deletionsFlag = 1;
                        c = fgetc(in);
                        charTracker = c;

                        if(c == '\n') {
                            if(deleteOverflowFlag) {
                                deleteCharCount = 0;
                                return c;
                            }
                            *(hunk_deletions_buffer + incrementDeleteBuffer) = 1;
                            
                            *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = c;
                            incrementDeleteBuffer++;
                            incrementDeleteBuffer = incrementDeleteBuffer + 2;
                            deleteCharCount = 0;

                                        if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
                            return c;
                        }

                        if(deleteOverflowFlag) {
                            return c;
                        }

                        *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = c;
                        incrementDeleteBuffer++;
                        deleteCharCount++;
                                    if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
                        return c;
                    }

                    else {
                        getcErrorFlag = 1;
                        return ERR;
                    }
                }

                else {
                    getcErrorFlag = 1;
                    return ERR;
                }
            }

            else if(c == '-') {
                c = fgetc(in);
                charTracker = c;

                if(c == '-') {
                    c = fgetc(in);
                    charTracker = c;

                    if(c == '-') {
                        c = fgetc(in);
                        charTracker = c;

                        if(c == '\n') {
                            //printf("%d\n7: ", addCharCount);
                            //printf("%d\n8: ", deleteCharCount);
        if(addCharCount != 0 && addOverflowFlag != 1) {
            *(hunk_additions_buffer + 2 + incrementAddBuffer) = '\n';
            *(hunk_additions_buffer + incrementAddBuffer - addCharCount) = (addCharCount + 1) % 256;
            *(hunk_additions_buffer + incrementAddBuffer - addCharCount + 1) = (addCharCount + 1) / 256;

            incrementAddBuffer++;
            incrementAddBuffer = incrementAddBuffer + 2;

            if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
        }

        else if(deleteCharCount != 0 && deleteOverflowFlag != 1) {
            *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = '\n';
            *(hunk_deletions_buffer + incrementDeleteBuffer - deleteCharCount) = (deleteCharCount + 1) % 256;
            *(hunk_deletions_buffer + incrementDeleteBuffer - deleteCharCount + 1) = (deleteCharCount + 1) / 256;

            incrementDeleteBuffer++;
            incrementDeleteBuffer = incrementDeleteBuffer + 2;

            if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
        }




                            addCharCount = 0;
                            deleteCharCount = 0;


                            firstChangeReadFlag = 1;
                            additionsFlag = 1;
                            deletionsFlag = 0;

                            EOSCount++;
                            return EOS;
                        }

                        else {
                            getcErrorFlag = 1;
                            return ERR;
                        }
                    }

                    else {
                        getcErrorFlag = 1;
                        return ERR;
                    }
                }

                else {
                    getcErrorFlag = 1;
                    return ERR;
                }
            }

            else if(c >= '0' && c <= '9') {
                ungetc(c, in);
                charTracker = tempChar;

                if(firstChangeReadFlag != 1) {
                    return ERR;
                }

                EOSCount++;
                return EOS;
            }

            else if(c == EOF) {

                if(firstChangeReadFlag != 1) {
                    return ERR;
                }
                EOSCount++;
                return EOS;
            }

            else {
                getcErrorFlag = 1;
                return ERR;
            }
        }

        else if(hp->type == HUNK_DELETE_TYPE) {
            if(c == '<') {
                c = fgetc(in);
                charTracker = c;

                if(c == ' ') {
                    deletionsFlag = 1;
                    c = fgetc(in);
                    charTracker = c;

                        if(c == '\n') {
                            if(deleteOverflowFlag) {
                                deleteCharCount = 0;
                                return c;
                            }
                            *(hunk_deletions_buffer + incrementDeleteBuffer) = 1;
                            
                            *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = c;
                            incrementDeleteBuffer++;
                            incrementDeleteBuffer = incrementDeleteBuffer + 2;
                            deleteCharCount = 0;

                                        if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
                            return c;
                        }

                    if(deleteOverflowFlag) {
                        return c;
                    }
                    *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = c;
                    incrementDeleteBuffer++;
                    deleteCharCount++;

                                if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
                deleteOverflowFlag = 1;
            }
                    return c;
                }

                else {
                    getcErrorFlag = 1;
                    return ERR;
                }
            }

            else if(c >= '0' && c <= '9') {
                ungetc(c, in);
                charTracker = tempChar;
                EOSCount++;
                return EOS;
            }

            else if(c == EOF) {
                EOSCount++;
                return EOS;
            }

            else {
                getcErrorFlag = 1;
                return ERR;
            }
        }

        else if(hp->type == HUNK_APPEND_TYPE) {
            if(c == '>') {
                c = fgetc(in);
                charTracker = c;

                if(c == ' ') {
                    additionsFlag = 1;
                    c = fgetc(in);
                    charTracker = c;

                    if(c == '\n') {
                        if(addOverflowFlag) {
                            addCharCount = 0;
                            return c;
                        }
                        *(hunk_additions_buffer + incrementAddBuffer) = 1;
                        *(hunk_additions_buffer + 2 + incrementAddBuffer) = c;
                        incrementAddBuffer++;
                        incrementAddBuffer = incrementAddBuffer + 2;
                        addCharCount = 0;
                                    if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
                        return c;
                    }

                    if(addOverflowFlag) {
                        return c;
                    }
                    *(hunk_additions_buffer + 2 + incrementAddBuffer) = c;
                    incrementAddBuffer++;
                    addCharCount++;

                                if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
                    return c;
                }

                else {
                    getcErrorFlag = 1;
                    return ERR;
                }
            }

            else if(c >= '0' && c <= '9') {
                ungetc(c, in);
                charTracker = tempChar;
                EOSCount++;
                return EOS;
            }

            else if(c == EOF) {
                EOSCount++;
                return EOS;
            }

            else {
                getcErrorFlag = 1;
                return ERR;
            }
        }

        else {
            getcErrorFlag = 1;
            return ERR;
        }
    }

    else {
        if(additionsFlag) {
            if(c == EOF) {
                return ERR;
            }

                                    if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }

            if(addOverflowFlag) {
                return c;
            }
            *(hunk_additions_buffer + 2 + incrementAddBuffer) = c;
            incrementAddBuffer++;
            addCharCount++;

                        if(HUNK_MAX - 2 - incrementAddBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementAddBuffer <= 4)) {
                addOverflowFlag = 1;
            }
            return c;
        }

        else if(deletionsFlag) {
            if(c == EOF) {
                return ERR;
            }

                            if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
        deleteOverflowFlag = 1;
    }

            if(deleteOverflowFlag) {
                return c;
            }
            
            *(hunk_deletions_buffer + 2 + incrementDeleteBuffer) = c;
            incrementDeleteBuffer++;
            deleteCharCount++;

                if(HUNK_MAX - 2 - incrementDeleteBuffer <= 2 || (charTracker == '\n' && HUNK_MAX - incrementDeleteBuffer <= 4)) {
        deleteOverflowFlag = 1;
    }


            return c;
        }

        else {
            getcErrorFlag = 1;
            return ERR;
        }
    }
    }
}

/**
 * @brief  Print a hunk to an output stream.
 * @details  This function prints a representation of a hunk to a
 * specified output stream.  The printed representation will always
 * have an initial line that specifies the type of the hunk and
 * the line numbers in the "old" and "new" versions of the file,
 * in the same format as it would appear in a traditional diff file.
 * The printed representation may also include portions of the
 * lines to be deleted and/or inserted by this hunk, to the extent
 * that they are available.  This information is defined to be
 * available if the hunk is the current hunk, which has been completely
 * read, and a call to hunk_next() has not yet been made to advance
 * to the next hunk.  In this case, the lines to be printed will
 * be those that have been stored in the hunk_deletions_buffer
 * and hunk_additions_buffer array.  If there is no current hunk,
 * or the current hunk has not yet been completely read, then no
 * deletions or additions information will be printed.
 * If the lines stored in the hunk_deletions_buffer or
 * hunk_additions_buffer array were truncated due to there having
 * been more data than would fit in the buffer, then this function
 * will print an elipsis "..." followed by a single newline character
 * after any such truncated lines, as an indication that truncation
 * has occurred.
 *
 * @param hp  Data structure giving the header information about the
 * hunk to be printed.
 * @param out  Output stream to which the hunk should be printed.
 */

void hunk_show(HUNK *hp, FILE *out) {
    if(hp->type == 0 || hp->serial == 0) {
        return;
    }

    if(hp->type == HUNK_CHANGE_TYPE) {
        if(hp->old_start != hp->old_end) {
            int convertChar = 1;
            int convertNum = hp->old_start;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;

            fputc(',', out);

            convertNum = hp->old_end;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;

            fputc('c', out);
        }

        else {
            int convertChar = 1;
            int convertNum = hp->old_start;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;
            fputc('c', out);
        }

        if(hp->new_start != hp->new_end) {
            int convertChar = 1;
            int convertNum = hp->new_start;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;

            fputc(',', out);

            convertNum = hp->new_end;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;
        }

        else {
            int convertChar = 1;
            int convertNum = hp->new_start;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;
        }

        fputc('\n', out);

        if(*(hunk_deletions_buffer) == 0) {
            return;
        }

        if(getcErrorFlag == 1 && EOSCount != 2) {
            return;
        }


        fputc('<', out);
        fputc(' ', out);


        for(int i = 2; i < HUNK_MAX - 2; i++) {
            if(*(hunk_deletions_buffer + i) == 0) {
                if(*(hunk_deletions_buffer + i + 1) == 0) {
                    break;
                }
            }

            fputc(*(hunk_deletions_buffer + i), out);

            if(*(hunk_deletions_buffer + i) == '\n') {
                if(*(hunk_deletions_buffer + i + 1) == 0) {
                    if(*(hunk_deletions_buffer + i + 2) == 0) {
                        break;
                    }
                }
                fputc('<', out);
                fputc(' ', out);
                i = i + 2;
            }
        }

        if(*(hunk_deletions_buffer + HUNK_MAX - 3) != 0) {
            fputc('.', out);
            fputc('.', out);
            fputc('.', out);
            fputc('\n', out);
        }

        fputc('-', out);
        fputc('-', out);
        fputc('-', out);

        fputc('\n', out);

        if(*(hunk_additions_buffer) == 0) {
            return;
        }

        fputc('>', out);
        fputc(' ', out);

        for(int i = 2; i < HUNK_MAX - 2; i++) {
            if(*(hunk_additions_buffer + i) == 0) {
                if(*(hunk_additions_buffer + i + 1) == 0) {
                    break;
                }
            }

            fputc(*(hunk_additions_buffer + i), out);

            if(*(hunk_additions_buffer + i) == '\n') {
                if(*(hunk_additions_buffer + i + 1) == 0) {
                    if(*(hunk_additions_buffer + i + 2) == 0) {
                        break;
                    }
                }
                fputc('>', out);
                fputc(' ', out);
                i = i + 2;
            }
        }

        if(*(hunk_additions_buffer + HUNK_MAX - 3) != 0) {
            fputc('.', out);
            fputc('.', out);
            fputc('.', out);
            fputc('\n', out);
        }
    }

    else if(hp->type == HUNK_APPEND_TYPE) {
        int convertChar = 1;
        int convertNum = hp->old_start;
        while (convertNum / convertChar >= 10) {
            convertChar *= 10;
        }
        while (convertChar > 0) {
            fputc((convertNum / convertChar) + '0', out);
            convertNum %= convertChar;
            convertChar /= 10;
        }
        convertChar = 1;

        fputc('a', out);

        if(hp->new_start != hp->new_end) {
            int convertChar = 1;
            int convertNum = hp->new_start;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;
            fputc(',', out);

            convertNum = hp->new_end;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;
        }

        else {
            int convertChar = 1;
            int convertNum = hp->new_start;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;
        }

        fputc('\n', out);

        if(*(hunk_additions_buffer) == 0) {
            return;
        }

        if(getcErrorFlag == 1 && EOSCount != 1) {
            return;
        }

        fputc('>', out);
        fputc(' ', out);

        for(int i = 2; i < HUNK_MAX - 2; i++) {
            if(*(hunk_additions_buffer + i) == 0) {
                if(*(hunk_additions_buffer + i + 1) == 0) {
                    break;
                }
            }

            fputc(*(hunk_additions_buffer + i), out);

            if(*(hunk_additions_buffer + i) == '\n') {
                if(*(hunk_additions_buffer + i + 1) == 0) {
                    if(*(hunk_additions_buffer + i + 2) == 0) {
                        break;
                    }
                }
                fputc('>', out);
                fputc(' ', out);
                i = i + 2;
            }
        }

        if(*(hunk_additions_buffer + HUNK_MAX - 3) != 0) {
            fputc('.', out);
            fputc('.', out);
            fputc('.', out);
            fputc('\n', out);
        }

    }

    else if(hp->type == HUNK_DELETE_TYPE) {
        if(hp->old_start != hp->old_end) {
            int convertChar = 1;
            int convertNum = hp->old_start;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;

            fputc(',', out);

            convertNum = hp->old_end;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;

            fputc('d', out);
        }

        else {
            int convertChar = 1;
            int convertNum = hp->old_start;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;
            fputc('d', out);
        }

        int convertChar = 1;
        int convertNum = hp->new_start;
            while (convertNum / convertChar >= 10) {
                convertChar *= 10;
            }
            while (convertChar > 0) {
                fputc((convertNum / convertChar) + '0', out);
                convertNum %= convertChar;
                convertChar /= 10;
            }
            convertChar = 1;

        fputc('\n', out);

        if(*(hunk_deletions_buffer) == 0) {
            return;
        }

        if(getcErrorFlag == 1 && EOSCount != 1) {
            return;
        }

        fputc('<', out);
        fputc(' ', out);

        for(int i = 2; i < HUNK_MAX - 2; i++) {
            if(*(hunk_deletions_buffer + i) == 0) {
                if(*(hunk_deletions_buffer + i + 1) == 0) {
                    break;
                }
            }

            fputc(*(hunk_deletions_buffer + i), out);

            if(*(hunk_deletions_buffer + i) == '\n') {
                if(*(hunk_deletions_buffer + i + 1) == 0) {
                    if(*(hunk_deletions_buffer + i + 2) == 0) {
                        break;
                    }
                }
                fputc('<', out);
                fputc(' ', out);
                i = i + 2;
            }
        }

        if(*(hunk_deletions_buffer + HUNK_MAX - 3) != 0) {
            fputc('.', out);
            fputc('.', out);
            fputc('.', out);
            fputc('\n', out);
        }
    }

}

/**
 * @brief  Patch a file as specified by a diff.
 * @details  This function reads a diff file from an input stream
 * and uses the information in it to transform a source file, read on
 * another input stream into a target file, which is written to an
 * output stream.  The transformation is performed "on-the-fly"
 * as the input is read, without storing either it or the diff file
 * in memory, and errors are reported as soon as they are detected.
 * This mode of operation implies that in general when an error is
 * detected, some amount of output might already have been produced.
 * In case of a fatal error, processing may terminate prematurely,
 * having produced only a truncated version of the result.
 * In case the diff file is empty, then the output should be an
 * unchanged copy of the input.
 *
 * This function checks for the following kinds of errors: ill-formed
 * diff file, failure of lines being deleted from the input to match
 * the corresponding deletion lines in the diff file, failure of the
 * line numbers specified in each "hunk" of the diff to match the line
 * numbers in the old and new versions of the file, and input/output
 * errors while reading the input or writing the output.  When any
 * error is detected, a report of the error is printed to stderr.
 * The error message will consist of a single line of text that describes
 * what went wrong, possibly followed by a representation of the current
 * hunk from the diff file, if the error pertains to that hunk or its
 * application to the input file.  If the "quiet mode" program option
 * has been specified, then the printing of error messages will be
 * suppressed.  This function returns immediately after issuing an
 * error report.
 *
 * The meaning of the old and new line numbers in a diff file is slightly
 * confusing.  The starting line number in the "old" file is the number
 * of the first affected line in case of a deletion or change hunk,
 * but it is the number of the line *preceding* the addition in case of
 * an addition hunk.  The starting line number in the "new" file is
 * the number of the first affected line in case of an addition or change
 * hunk, but it is the number of the line *preceding* the deletion in
 * case of a deletion hunk.
 *
 * @param in  Input stream from which the file to be patched is read.
 * @param out Output stream to which the patched file is to be written.
 * @param diff  Input stream from which the diff file is to be read.
 * @return 0 in case processing completes without any errors, and -1
 * if there were errors.  If no error is reported, then it is guaranteed
 * that the output is complete and correct.  If an error is reported,
 * then the output may be incomplete or incorrect.
 */

int patch(FILE *in, FILE *out, FILE *diff) {
    int lineCounter = 0;
    int charFromInput = 0;
    int charFromHunk = 0;
    int inputLineCount = 1;

    int lineChecker = 0;

    HUNK hunk;
    hunk.type = 0;
    hunk.old_start = 0;
    hunk.old_end = 0;
    hunk.new_start = 0;
    hunk.new_end = 0;
    hunk.serial = 0;

    int nextReturns = hunk_next(&hunk, diff);

    while(nextReturns != EOF) {
        lineChecker = 0;

        if(nextReturns == ERR) {
            if(global_options & QUIET_OPTION) {
                return -1;
            }
            fprintf(stderr, "Error! The program was not able to get the next hunk header because of either an error in the formatting of the header or an error in the contents of the previous hunk.");
            return -1;
        }

        if(inputLineCount - 1 > hunk.old_start) {
            if(global_options & QUIET_OPTION) {
                return -1;
            }
            fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
            hunk_show(&hunk, stderr);
            return -1;
        }

        int linesToRead = 1 + hunk.old_end - hunk.old_start;

        if(hunk.type == HUNK_CHANGE_TYPE || hunk.type == HUNK_DELETE_TYPE) {
            int lineSkipper = hunk.old_start;

            while(inputLineCount != lineSkipper) {
                charFromInput = fgetc(in);

                if(charFromInput == EOF) {
                    if(global_options & QUIET_OPTION) {
                        return -1;
                    }
                    fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
                    hunk_show(&hunk, stderr);
                    return -1;
                }

                if(charFromInput == '\n') {
                    inputLineCount++;
                }

                if(!(global_options & NO_PATCH_OPTION)) {
                    fputc(charFromInput, out);
                }
            }
        }


        if(hunk.type == HUNK_APPEND_TYPE) {
            int lineSkipper = hunk.old_start + 1;

            while(inputLineCount != lineSkipper) {
                charFromInput = fgetc(in);

                if(charFromInput == EOF) {
                    if(global_options & QUIET_OPTION) {
                        return -1;
                    }
                    fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
                    hunk_show(&hunk, stderr);
                    return -1;
                }

                if(charFromInput == '\n') {
                    inputLineCount++;
                }

                if(!(global_options & NO_PATCH_OPTION)) {
                    fputc(charFromInput, out);
                }
            }
        }

        if(hunk.type == HUNK_CHANGE_TYPE) {
            charFromHunk = hunk_getc(&hunk, diff);

            while(charFromHunk != EOS) {
                charFromInput = fgetc(in);

                if(charFromInput == charFromHunk) {
                    //
                }

                else {
                    if(global_options & QUIET_OPTION) {
                        return -1;
                    }
                    fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
                    hunk_show(&hunk, stderr);
                    return -1;
                }

                if(charFromInput == '\n') {
                    lineChecker++;
                    inputLineCount++;
                }

                charFromHunk = hunk_getc(&hunk, diff);
            }


            if(lineChecker != hunk.old_end - hunk.old_start + 1) {
                if(global_options & QUIET_OPTION) {
                    return -1;
                }
                fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
                hunk_show(&hunk, stderr);
                return -1;
            }

            lineChecker = 0;


            charFromHunk = hunk_getc(&hunk, diff);

            while(charFromHunk != EOS) {
                if(charFromHunk == ERR) {
                    if(global_options & QUIET_OPTION) {
                        return -1;
                    }
                    fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
                    hunk_show(&hunk, stderr);
                    return -1;
                }

                if(!(global_options & NO_PATCH_OPTION)) {
                    fputc(charFromHunk, out);
                }

                if(charFromHunk == '\n') {
                    lineChecker++;
                    //inputLineCount++;
                }

                charFromHunk = hunk_getc(&hunk, diff);
            }

            if(lineChecker != hunk.new_end - hunk.new_start + 1) {
                if(global_options & QUIET_OPTION) {
                    return -1;
                }
                fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
                hunk_show(&hunk, stderr);
                return -1;
            }

        }

        else if(hunk.type == HUNK_APPEND_TYPE) {
            int counter = 0;

            charFromHunk = hunk_getc(&hunk, diff);

            while(charFromHunk != EOS) {
                if(charFromHunk == ERR) {
                    if(global_options & QUIET_OPTION) {
                        return -1;
                    }
                    fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
                    hunk_show(&hunk, stderr);
                    return -1;
                }

                if(!(global_options & NO_PATCH_OPTION)) {
                    fputc(charFromHunk, out);
                }

                if(charFromHunk == '\n') {
                    //inputLineCount++;
                    lineChecker++;
                    counter++;
                }

                charFromHunk = hunk_getc(&hunk, diff);

            }

            if(lineChecker != hunk.new_end - hunk.new_start + 1) {
                if(global_options & QUIET_OPTION) {
                    return -1;
                }
                fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
                hunk_show(&hunk, stderr);
                return -1;
            }
        }

        else if(hunk.type == HUNK_DELETE_TYPE) {
            int counter = 0;

            charFromHunk = hunk_getc(&hunk, diff);

            while(charFromHunk != EOS) {

                charFromInput = fgetc(in);

                if(charFromInput == charFromHunk) {
                    //
                }

                else {
                    if(global_options & QUIET_OPTION) {
                        return -1;
                    }
                    fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
                    hunk_show(&hunk, stderr);
                    return -1;
                }

                if(charFromInput == '\n') {
                    lineChecker++;
                    inputLineCount++;
                    counter++;
                }

                charFromHunk = hunk_getc(&hunk, diff);
            }

            if(lineChecker != hunk.old_end - hunk.old_start + 1) {
                if(global_options & QUIET_OPTION) {
                    return -1;
                }
                fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
                hunk_show(&hunk, stderr);
                return -1;
            }
        }

        else {
            if(global_options & QUIET_OPTION) {
                return -1;
            }
            fprintf(stderr, "Error! The program detected an error in the formatting of the hunk. The contents of the hunk that caused this error can be seen below:\n");
            hunk_show(&hunk, stderr);
            return -1;
        }

        nextReturns = hunk_next(&hunk, diff);
    }

    charFromInput = fgetc(in);

    while(charFromInput != EOF) {
        if(!(global_options & NO_PATCH_OPTION)) {
            fputc(charFromInput, out);
        }
        charFromInput = fgetc(in);
    }

    return 0;
}
