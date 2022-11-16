/* FIX THIS

- input1-3.txt keyword is in the 2th line but it prints as 3rd

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#define BUFFER_SIZE 209715200 // 200 MB
#define ROOT_DIR "./"
#define BUFFER_DIR "buffer_"
#define PIPE_BUFFER 2063
#define OUTPUT_BUFFER_SIZE 10240
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

    /* CREATE PIPES */
    int pfds[filesCount][2];

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
        if (pipe(pfds[c]) < 0)
        {
            perror("Error opening pipe!\n");
            exit(-1);
        }

        pids[c] = fork(); /* FORK A PROCESS */

        char currentInputFileDir[WORD_BUFFER];
        char *fileDir = inputFiles[global_fileCounter];
        char *bufferOutputDir = malloc(WORD_BUFFER);

        // Make a complete dir for the argv[i]

        // TODO: change with sprintf
        // strcat(currentInputFileDir, ROOT_DIR);
        // strcat(currentInputFileDir, fileDir);
        sprintf(currentInputFileDir, "%s%s", ROOT_DIR, fileDir);
        // printf("Global counter: %d\n", global_fileCounter);

        // create directories for buffer output files
        // TODO: change with sprintf
        // strcat(bufferOutputDir, ROOT_DIR);                       /* ./ */
        // strcat(bufferOutputDir, BUFFER_DIR);                     /* ./buffer_ */
        // strcat(bufferOutputDir, inputFiles[global_fileCounter]); /* ./buffer_input1.txt*/
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
                perror("Current input file is NULL\n");
                return 0;
            }

            // Open file content
            if ((inputFileStream = fopen(currentInputFileDir, "r")) == NULL)
            {
                printf("Error opening %s\n", currentInputFileDir);
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
            char line[LINE_BUFFER] = {0x0};

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

            int l = 0;
            char msg[PIPE_BUFFER];

            // printf("Message sent to PIPE:\n");
            for (int k = 0; k < MAX_MATCHED_LINES; k++)
            {
                if (k == matchedLinesIndices[l])
                {
                    strcat(msg, currentInputFileDir);
                    strcat(msg, ", ");
                    char str_k[2];
                    sprintf(str_k, "%d", k);
                    strcat(msg, str_k);
                    strcat(msg, ": ");
                    strcat(msg, matchedLines[k]);
                    strcat(msg, "\n");
                    /* COMPOSE PIPE MESSAGE TO BE SENT */
                    // sprintf(msg, "%s, %d: %s\n", currentInputFileDir, k, matchedLines[k]);
                    // printf("%s\n", msg);
                    //  fprintf(bufferOutputFileStream, "%s, %d: %s\n", currentInputFileDir, k, matchedLines[k]);
                    l++;
                }
            }

            if (write(pfds[c][1], msg, sizeof(msg)) == -1)
            {
                perror("Error writing pipe!\n");
                exit(-1);
            }
            /* WRITE TO PIPE */
            close(pfds[c][1]); /* CLOSE WRITING TO PIPE */
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

    // READ PIPES
    char total_msg[PIPE_BUFFER] = {0x0};
    for (int i = 0; i < filesCount; i++)
    {
        close(pfds[i][1]); /* CLOSE WRITING TO PIPE */
        char msg[WORD_BUFFER] = {0x0};
        if (read(pfds[i][0], msg, sizeof(msg)) == -1)
        {
            perror("Error reading pipe message!\n");
            return 1;
        }

        strcat(total_msg, msg);
        strcat(total_msg, "\n");
        close(pfds[i][0]); /* CLOSE READING PIPE */
    }

    // printf("Received message from PIPE:\n%s\n", total_msg);

    // /* WRITE TO THE OUTPUT FILE */
    FILE *outputFileStream;

    char outputDir[WORD_BUFFER] = {0x0};

    strcat(outputDir, ROOT_DIR);
    strcat(outputDir, outputFileName);
    // printf("Output dir: %s \n", outputDir); /* DEBUG */
    if ((outputFileStream = fopen(outputDir, "w")) == NULL)
    {
        perror("Error opening output.txt file: \n");
        exit(-1);
    }

    fprintf(outputFileStream, "%s", total_msg);
    fclose(outputFileStream);

    /* REMOVE BUFFER OUTPUTS */
   
    return 0;
}