/* FIX THIS

- input13.txt keyword is in the 2th line but it prints as 3rd

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ROOT_DIR "./"
#define BUFFER_SIZE 209715200 // 200 MB
#define WORD_BUFFER 1024
#define LINE_BUFFER 1024
#define MAX_MATCHED_LINES 1024
#define FOUND 1
#define NOT_FOUND 0

int main(int argc, int **argv)
{

    if (argc < 1)
    {
        printf("Number of input files must be greater than 1!\n");
        return 0;
    }

    // GET arguments
    char *searchKeyword = argv[1];  /* DO NOT CHANGE: reserved for the keyword*/
    int filesCount = atoi(argv[2]); /* DO NOT CHANGE: reserved for the files count*/

    // Store all file names in array
    char *textFiles[1024];
    int e = 0;
    for (int d = 3; d < argc - 1; d++)
    {
        textFiles[e] = argv[d];
        e++;
    }

    /* DEBUGGING: Print input file names*/
    // for (int i = 0; i < 1024; i++)
    // {
    //     if (i == 30)
    //     {
    //         printf("\n");
    //     }

    //     printf("%s", textFiles[i]);

    // }

    /* CREATE PROCESSES */
    int pids[filesCount]; /* store process ids */

    int global_fileCounter = 0;
    for (int c = 0; c < filesCount; c++)
    {
        // printf("Global counter: %d\n", global_fileCounter);

        char currentInputFileDir[WORD_BUFFER];
        char *fileDir = textFiles[global_fileCounter];

        if (currentInputFileDir == NULL)
        {
            printf("No memory\n");
            return 0;
        }

        // Make a complete dir for the argv[i]
        strcat(currentInputFileDir, ROOT_DIR);
        strcat(currentInputFileDir, fileDir);

        printf("CURRENT FILE: %s\n", currentInputFileDir);

        /* FORK */
        pids[c] = fork();

        if (pids[c] < 0)
        {
            printf("Couldn\'t create child process.\n");
            exit(-1);
        }

        else if (pids[c] == 0) /* CHILD PROCESS */
        {
            char *bufferOutputDir = malloc(1024);

            // create directories for buffer output files
            strcat(bufferOutputDir, ROOT_DIR);                      /* ./ */
            strcat(bufferOutputDir, "buffer_");                     /* ./buffer- */
            strcat(bufferOutputDir, textFiles[global_fileCounter]); /* ./buffer_input1.txt*/

            // Start reading input file
            FILE *inputFileStream; /* single stream for a file */

            // Open file content
            if ((inputFileStream = fopen(currentInputFileDir, "r")) == NULL)
            {
                printf("Error opening input file!\n");
                return 0;
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

            // Write all lines to an array
            rewind(inputFileStream);

            int j = 1;
            while ((fgets(line, LINE_BUFFER, inputFileStream)) != NULL)
            {
                strcpy(matchedLines[j], line);
                j++;
            }

            // End of reading file
            rewind(inputFileStream);
            fclose(inputFileStream);

            // Write to output file
            FILE *outputFileStream;

            if ((outputFileStream = fopen(bufferOutputDir, "a")) == NULL)
            {
                printf("Error opening output file!\n");
                return 0;
            }

            int l = 0;
            for (int k = 0; k < MAX_MATCHED_LINES; k++)
            {

                // TODO: write to buffer file
                if (k == matchedLinesIndices[l])
                {
                    fprintf(outputFileStream, "%s, %d: %s\n", currentInputFileDir, k, matchedLines[k]);
                    l++;
                }
            }

            fclose(outputFileStream);
            rewind(inputFileStream);
            rewind(outputFileStream);

            exit(0);
        }

        else /* PARENT PROCESS */
        {
            global_fileCounter++;
            // TODO: write to output.txt
            *currentInputFileDir = '\0';
        }
    }

    for (int c = 0; c < filesCount; c++)
    {
        wait(NULL);
    }

    return 0;
}