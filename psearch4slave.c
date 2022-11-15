/*
    WRITE TO SEMAPHORE
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>

#define ROOT_DIR "./"
#define WORD_BUFFER 1024
#define LINE_BUFFER 3072
#define MAX_MATCHED_LINES 1024
#define SHM_BUFFER 4096
#define SEM_FNAME "semaphore"
#define SH_FNAME "/dev/null"

int main(int argc, int **argv)
{
    if (argc < 1)
    {
        printf("[SLAVE] Number of input files must be greater than 1!\n");
        return 0;
    }

    char *searchKeyword = argv[1];
    char *inputFile = argv[2];
    char currentInputFileDir[100];

    // Make a complete directory for the input file
    sprintf(currentInputFileDir, "%s%s", ROOT_DIR, inputFile);

    // printf("Current file dir: %s\n", currentInputFileDir);
    if (currentInputFileDir == NULL)
    {
        perror("[SLAVE] Current file directory is NULL!\n");
        return 1;
    }

    FILE *inputFileStream; /* single stream for a file */

    // Open file content
    if ((inputFileStream = fopen(currentInputFileDir, "r")) == NULL)
    {
        perror("[SLAVE] Error opening input file!\n");
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
    char word[WORD_BUFFER] = {0x0};
    int wordChCount = 0;

    /* READ FILE AND FIND KEYWORD */
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
    } /* end -- READ FILE AND FIND KEYWORD */

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

    /*
        CREATE THE MESSAGE with format:
        <input_file>, <matched_line_index>: <matched_line>
    */
    int l = 0;
    char msg[SHM_BUFFER] = {0x0};

    /* COMPOSE A MESSAGE*/
    for (int k = 0; k < MAX_MATCHED_LINES; k++)
    {
        if (k == matchedLinesIndices[l])
        {
            char line[LINE_BUFFER] = {0x0};
            sprintf(line, "%s, %d: %s\n", currentInputFileDir, k, matchedLines[k]);
            strcat(msg, line);
            l++;
        }
    }

    // printf("Composing message: %s\n", msg);

       char *shrd_value = shmat(shm_id, NULL, 0);
    sem_t *sem = sem_open(SEM_FNAME, O_CREAT | O_EXCL, 0644, sem_value);

    /* WRITE TO SHARED MEMORY WITH SYNCHRONIZATION */
    sem_wait(sem_cons);
    sprintf(shrd_value, "%s", msg);
    printf("[SLAVE] Writing %s\n", shdmem);
    sem_post(sem_prod);

    return 0;
}