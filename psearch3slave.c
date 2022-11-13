#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define ROOT_DIR            "./"
#define OUTPUT_BUFFER_SIZE  10240 /* 10 KB */
#define WORD_BUFFER         1024  /* 1 KB */
#define LINE_BUFFER         1024
#define MAX_MATCHED_LINES   1024
#define SMEM_BUFFER         1024

int main(int argc, int **argv)
{

    if (argc < 1)
    {
        printf("Number of input files must be greater than 1!\n");
        return 0;
    }

    char *searchKeyword = argv[1];
    char *inputFile = argv[2];
    char currentInputFileDir[100];

    // Make a complete directory for the input file
    sprintf(currentInputFileDir, "%s%s", ROOT_DIR, inputFile);

    //printf("Current file dir: %s\n", currentInputFileDir);
    if (currentInputFileDir == NULL)
    {
        perror("Current file directory is NULL!\n");
        return 1;
    }

    FILE *inputFileStream; /* single stream for a file */

    // Open file content
    if ((inputFileStream = fopen(currentInputFileDir, "r")) == NULL)
    {
        perror("Error opening input file!\n");
        exit(-1);
    }
    

    // Create matched lines indices array
    int matchedLinesIndices[MAX_MATCHED_LINES];
    for (int b = 0; b < MAX_MATCHED_LINES; b++)
    {
        matchedLinesIndices[b] = 0;
    }

    int matchedLinesCount = 0;
    int myChar;
    int lineCount = 0;
    char word[WORD_BUFFER];
    int wordChCount = 0;

    // Read file by character to find matched line indices
    while (((myChar = fgetc(inputFileStream)) != EOF))
    {
        if (myChar == '\n')
        {
            lineCount++;
        }

        // Before beginning a new word
        if (myChar == ' ' || myChar == '\n' || myChar == '\0' || myChar == '\t')
        {
            // Is it the keyword?
            if (strcmp(word, searchKeyword) == 0)
            {
                matchedLinesIndices[matchedLinesCount] = lineCount + 1;
                matchedLinesCount++;
            }

            // Reset word
            word[0] = '\0';
            wordChCount = 0;
        }
        else
        {
            word[wordChCount++] = (char)myChar;
            word[wordChCount] = '\0';
        }
    }

    char matchedLines[MAX_MATCHED_LINES][WORD_BUFFER];
    char line[LINE_BUFFER];

    // Reset input stream
    rewind(inputFileStream);

    // GET lines by the matched lines indices
    int j = 1;
    while ((fgets(line, LINE_BUFFER, inputFileStream)) != NULL)
    {
        strcpy(matchedLines[j], line);
        j++;
    }

    fclose(inputFileStream);

    /*
        CREATE THE MESSAGE with format:
        <input_file>, <matched_line_index>: <matched_line>
    */
    int l = 0;
    char msg[SMEM_BUFFER];

    //printf("Message sent to SHARED MEMORY:\n");
    /* COMPOSE PIPE MESSAGE TO BE SENT */
    for (int k = 0; k < MAX_MATCHED_LINES; k++)
    {
        if (k == matchedLinesIndices[l])
        {   
            //TODO: change with sprintf
            strcat(msg, currentInputFileDir);
            strcat(msg, ", ");
            char str_k[2];
            sprintf(str_k, "%d", k);
            strcat(msg, str_k);
            strcat(msg, ": ");
            strcat(msg, matchedLines[k]);
            strcat(msg, "\n");
            
            // sprintf(msg, "%s, %d: %s\n", currentInputFileDir, k, matchedLines[k]);
            //printf("%s\n", msg);
            //  fprintf(bufferOutputFileStream, "%s, %d: %s\n", currentInputFileDir, k, matchedLines[k]);
            l++;
        }

        // TODO: write message to shared memory
    }
    return 0;
}