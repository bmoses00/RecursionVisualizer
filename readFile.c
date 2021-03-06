#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ASSUMES that all function declarations are single line and have one int as input

int comparator(const void *a, const void *b) {
    int *x = (int *)a;
    int *y = (int *)b;
    return *x - *y;
}

int main() {
    FILE* f = popen("ctags -x --c-kinds=f recurse.c", "r");

    char buffer[1024] = { 0 };
    char* buffer_ptr = buffer;
    fread(buffer, sizeof buffer, 1, f);
    pclose(f);

    int i, count;
    for (i = 0, count = 0; buffer[i]; i++)
        count += (buffer[i] == '\n');
    
    int array[count]; // array size should be equal to the number of lines in ctags output
    char* line = strtok_r(buffer, "\n", &buffer_ptr);
    for (int index = 0; index  < count; index++) {
        char* partOfLine = NULL;
        for (i = 0; i < 3; i++) // we want the third element of the ctags output
            partOfLine = strtok_r(line, " ", &line);
        array[index] = atoi(partOfLine);
        line = strtok_r(NULL, "\n", &buffer_ptr);
    }

    qsort(array, count, sizeof(int), comparator);

    FILE *fp;
    FILE *fOut;
    char *inputLine = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("./recurse.c", "r");
    fOut = fopen("modified.c", "w");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    int lineNo = 1;
    int functionNo = 0;
    while ((read = getline(&inputLine, &len, fp)) != -1) {
        fputs(inputLine, fOut);

        if (array[functionNo] == lineNo) {
            char* endOfParamsPtr = strchr(inputLine, ')');
            if (*(endOfParamsPtr - 1) == '(') {
                fputs("\tprintf(\"%s\", \"No arguments\\n\");\n", fOut);
            }
            else {
                *endOfParamsPtr = 0;
                int endOfParams = endOfParamsPtr - inputLine;
                int beginningOfParam = endOfParams;

                for (char currentChar = *endOfParamsPtr; currentChar != ' '; endOfParamsPtr--, currentChar = *endOfParamsPtr, beginningOfParam--);

                char* printStatementBeginning = "\tprintf(\"int ";
                char* printStatementEnd = ": %d\\n\", input);\n";

                char toPrint[strlen(printStatementBeginning) + strlen(beginningOfParam + inputLine + 1) + strlen(printStatementEnd)];

                strcpy(toPrint, printStatementBeginning);
                strcat(toPrint, beginningOfParam + inputLine + 1);
                strcat(toPrint, printStatementEnd);

                fputs(toPrint, fOut); 
                }
            functionNo++;
            }
        lineNo++;

        }
    

    fclose(fp);
    fclose(fOut);

    if (inputLine)
        free(inputLine);
    exit(EXIT_SUCCESS);
    return 0;
}