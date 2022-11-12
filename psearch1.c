/* FIX THIS

- cannot read input1.txt
- input13.txt keyword is in the 2th line but it prints as 3rd

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#define BUFFER_SIZE 209715200 // 200 MB
#define ROOT_DIR "./"
#define BUFFER_DIR "buffer_"
#define OUTPUT_BUFFER_SIZE 10240
#define WORD_BUFFER 1024
#define LINE_BUFFER 1024
#define MAX_MATCHED_LINES 1024

int main(int argc, int **argv)
{

    if (argc < 1)
    {
        printf("Number of input files must be greater than 1!\n");
        return 0;
    }

    // GET arguments
    char *searchKeyword = argv[1];         /* DO NOT CHANGE: reserved for the keyword*/
    int filesCount = atoi(argv[2]);        /* DO NOT CHANGE: reserved for the files count*/
    char *outputFileName = argv[argc - 1]; /* DO NOT CHANGE: reserved for the output file*/
    char *bufferOutputDirs[WORD_BUFFER];

    // Store all file names in array
    char *inputFiles[1024];
    int e = 0;
    for (int d = 3; d < argc - 1; d++)
    {
        inputFiles[e] = argv[d];
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

        pids[c] = fork(); /* FORK A PROCESS */

        char currentInputFileDir[WORD_BUFFER];
        char *fileDir = inputFiles[global_fileCounter];
        char *bufferOutputDir = malloc(WORD_BUFFER);

        // Make a complete dir for the argv[i]
        strcat(currentInputFileDir, ROOT_DIR);
        strcat(currentInputFileDir, fileDir);

        // printf("Global counter: %d\n", global_fileCounter);

        // create directories for buffer output files
        strcat(bufferOutputDir, ROOT_DIR);                       /* ./ */
        strcat(bufferOutputDir, BUFFER_DIR);                     /* ./buffer_ */
        strcat(bufferOutputDir, inputFiles[global_fileCounter]); /* ./buffer_input1.txt*/

        /* Store output dir in an array for convenience */
        bufferOutputDirs[global_fileCounter] = bufferOutputDir;

        if (pids[c] < 0)
        {
            printf("Couldn\'t create child process.\n");
            exit(-1);
        }
        else if (pids[c] == 0) /* CHILD PROCESS */
        {

            FILE *inputFileStream; /* single stream for a file */

            //printf("CURRENT INPUT FILE: %s\n", currentInputFileDir);

            if (currentInputFileDir == NULL)
            {
                printf("No memory\n");
                return 0;
            }

            // Open file content
            if ((inputFileStream = fopen(currentInputFileDir, "r")) == NULL)
            {
                printf("Error opening %s", currentInputFileDir);
                perror("Stack trace:\n");
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

            // Write all lines to an array
            rewind(inputFileStream);

            int j = 1;
            while ((fgets(line, LINE_BUFFER, inputFileStream)) != NULL)
            {
                strcpy(matchedLines[j], line);
                j++;
            }

            fclose(inputFileStream);

            // Write to output file
            FILE *bufferOutputFileStream;

            if ((bufferOutputFileStream = fopen(bufferOutputDir, "a")) == NULL)
            {
                perror("Error opening output file!\n");
                return 0;
            }

            int l = 0;
            for (int k = 0; k < MAX_MATCHED_LINES; k++)
            {

                // TODO: write to buffer file
                if (k == matchedLinesIndices[l])
                {
                    fprintf(bufferOutputFileStream, "%s, %d: %s\n", currentInputFileDir, k, matchedLines[k]);
                    l++;
                }
            }

            fclose(bufferOutputFileStream);

            exit(0);
        }

        else /* PARENT PROCESS */
        {
            global_fileCounter++;
            *currentInputFileDir = '\0';
        }
    }

    /* WAIT FOR CHILD PROCESSES TO FINISH*/
    for (int c = 0; c < filesCount; c++)
    {
        wait(NULL);
    }

    /* READ BUFFER OUTPUTS */
    char outputBuffer[OUTPUT_BUFFER_SIZE];
    int outputCharCount = 0;
    for (int i = 0; i < filesCount; i++)
    {
        FILE *bufferFileStream;
        if ((bufferFileStream = fopen(bufferOutputDirs[i], "r")) != NULL)
        {

            int outputChar;
            while (((outputChar = fgetc(bufferFileStream)) != EOF))
            {
                outputBuffer[outputCharCount++]S = (char)outputChar;
            }
        }

        fclose(bufferFileStream);
    }

    printf("Content:\n%s", outputBuffer); /* DEBUGGSING */

    /* WRITE TO THE OUTPUT FILE */
    FILE *outputFileStream;

    char *outputDir = malloc(WORD_BUFFER);

    strcat(outputDir, ROOT_DIR);
    strcat(outputDir, outputFileName);
    // printf("Output dir: %s \n", outputDir);
    if ((outputFileStream = fopen(outputDir, "w")) == NULL)
    {
        perror("Error opening output.txt file: \n");
        exit(-1);
    }

    fputs(outputBuffer, outputFileStream);
    fclose(outputFileStream);

    /* REMOVE BUFFER OUTPUTS */
    

    return 0;
}