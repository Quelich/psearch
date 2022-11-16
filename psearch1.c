#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>


#define ROOT_DIR "./"
#define BUFFER_DIR "buffer_"
#define OUTPUT_BUFFER_SIZE 10240 /* 10 mb */
#define WORD_BUFFER 1024
#define LINE_BUFFER 1024
#define MAX_MATCHED_LINES 1024
#define MAX_FILE_NUMBER 20

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
    char bufferOutputDirs[MAX_FILE_NUMBER][WORD_BUFFER];

    // SAVE all file names in array
    char *inputFiles[MAX_FILE_NUMBER];
    int e = 0;
    for (int d = 3; d < argc - 1; d++)
    {
        inputFiles[e] = argv[d];
        e++;
    }

    /* CREATE PROCESSES */
    int pids[filesCount]; /* store process ids */

    int global_fileCounter = 0;

    // ADD buffer files to a list
    for (int i = 0; i < filesCount; i++)
    {
        char bufferOutputDir[WORD_BUFFER];
        sprintf(bufferOutputDir, "%s%s%s", ROOT_DIR, BUFFER_DIR, inputFiles[i]);
        strcpy(bufferOutputDirs[i], bufferOutputDir);
        // printf("ADDED: %s into output list\n", bufferOutputDirs[global_fileCounter]); /* DEBUG */
    }

    for (int c = 0; c < filesCount; c++)
    {

        pids[c] = fork(); /* FORK A PROCESS */

        char currentInputFileDir[WORD_BUFFER] = {0x0};
        char *fileDir = inputFiles[global_fileCounter];
        char bufferOutputDir[WORD_BUFFER] = {0x0};

        // Make a complete dir for the argv[i]
        sprintf(currentInputFileDir, "%s%s", ROOT_DIR, fileDir);
        // create directories for buffer output files
        sprintf(bufferOutputDir, "%s%s%s", ROOT_DIR, BUFFER_DIR, inputFiles[global_fileCounter]);

        /* Store output dir in an array for convenience */

        if (pids[c] < 0)
        {
            printf("Couldn\'t create child process.\n");
            exit(-1);
        }

        else if (pids[c] == 0) /* CHILD PROCESS */
        {

            FILE *inputFileStream; /* single stream for a file */

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

            int lcounter = 0;
            char line[LINE_BUFFER];
            FILE *bufferOutputFileStream;
            if ((bufferOutputFileStream = fopen(bufferOutputDir, "w")) == NULL)
            {
                perror("Error opening output file!\n");
                return 0;
            }

            while (fgets(line, sizeof(line), inputFileStream) != NULL) // read throguh the file and store each line in lines_of_file
            {
                char *temp = line;
                lcounter++; // increment counter in each line
                if ((temp = strstr(line, searchKeyword)) != NULL)
                {
                    char *whitespace = temp + strlen(searchKeyword);

                    if (temp == line || isblank((unsigned char)*(temp - 1)))
                    {
                        /* if the exact keyword found in a line */
                        if (*whitespace == '\0' || isblank((unsigned char)*whitespace))
                        {
                            fprintf(bufferOutputFileStream, "%s, %d: %s", fileDir, lcounter, line);
                        }
                    }
                    temp = whitespace;
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
    char *outputBuffer = malloc(OUTPUT_BUFFER_SIZE);
    int outputCharCount = 0;
    int outputChar;

    for (int i = 0; i < filesCount; i++)
    {
        FILE *bufferFileStream;
        // printf("Current output buffer: %s\n", bufferOutputDirs[i]);
        if ((bufferFileStream = fopen(bufferOutputDirs[i], "r")) != NULL)
        {

            while (((outputChar = fgetc(bufferFileStream)) != EOF))
            {
                outputBuffer[outputCharCount++] = (char)outputChar;
            }
        }

        fclose(bufferFileStream);
    }

    /* WRITE TO THE OUTPUT FILE */
    FILE *outputFileStream;

    char outputDir[WORD_BUFFER] = {0x0};

    sprintf(outputDir, "%s%s", ROOT_DIR, outputFileName);

    if ((outputFileStream = fopen(outputDir, "w")) == NULL)
    {
        perror("Error opening output.txt file: \n");
        exit(-1);
    }

    fprintf(outputFileStream, "%s", outputBuffer);
    fclose(outputFileStream);

    /* REMOVE BUFFER OUTPUTS */
    for (int i = 0; i < filesCount; i++)
    {
        if (remove(bufferOutputDirs[i]) != 0)
        {
            printf("Unable to remove %s\n", bufferOutputDirs[i]);
        }
    }

    return 0;
}