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
#define SHM_BUFFER 2048 /* 10 mb */
#define SEM_PROD_FNAME "producer"
#define SEM_CONS_FNAME "consumer"
#define SH_FNAME "/dev/null"
#define FD_FNAME "semaphore"

int main(int argc, int **argv)
{

    sem_unlink(SEM_PROD_FNAME);
    sem_unlink(SEM_CONS_FNAME);
    if (argc < 1)
    {
        perror("[SLAVE] Number of input files must be greater than 1!\n");
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
    char msg[SHM_BUFFER] = {0x0}; /* MESSAGE TO SEND PARENT PROCESS */

    /* COMPOSE A MESSAGE*/
    for (int k = 0; k < MAX_MATCHED_LINES; k++)
    {
        if (k == matchedLinesIndices[l])
        {
            char line[LINE_BUFFER] = {0x0};
            sprintf(line, "%s, %d: %s", inputFile, k, matchedLines[k]);
            strcat(msg, line);
            l++;
        }
    }

    // printf("Composing message: %s\n", msg);
    // key_t shm_key;
    // int shm_id;
    // unsigned int sem_value;

    // shm_key = ftok(SH_FNAME, 0);

    // if (shm_key == -1)
    // {
    //     perror("[SLAVE]\nError creating shm_key\n");
    //     exit(EXIT_FAILURE);
    // }

    // shm_id = shmget(shm_key, sizeof(int), 0644 | IPC_CREAT);

    // if (shm_id < 0)
    // {
    //     perror("[SLAVE]\nError creating shm_id\n");
    //     exit(-1);
    // }

    int fd = shm_open(FD_FNAME, O_CREAT | O_RDWR, 0666);
    if (ftruncate(fd, SHM_BUFFER) == -1)
    {
        perror("ftruncate");
    }
    char *memblock = (char *)mmap(0, SHM_BUFFER, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // char *memblock = shmat(shm_id, NULL, 0);

    sem_t *prod = sem_open(SEM_PROD_FNAME, O_CREAT | O_EXCL, 0644, 0);

    if (prod == SEM_FAILED)
    {
        perror("[SLAVE]\nsemaphore/producer");
        exit(EXIT_FAILURE);
    }

    sem_t *cons = sem_open(SEM_CONS_FNAME, O_CREAT | O_EXCL, 0644, 1);

    if (cons == SEM_FAILED)
    {
        perror("[SLAVE]\nsemaphore/consumer");
        // exit(EXIT_FAILURE);
    }

    /* WRITE TO SHARED MEMORY WITH SYNCHRONIZATION */
    sem_wait(cons);
    // sleep(1); /* WARNING: IF YOU DO NOT WAIT CONSUMER, THE OUTPUT WILL BE INCOMPLETE*/
    strcat(memblock, msg);
    // printf("[SLAVE]\n Writing \n%s\n", memblock);
    sem_post(prod);

    /* DEATTACH MEMORY */
    shmdt(memblock);
    // shmctl(shm_id, IPC_RMID, 0);
    sem_close(prod);
    sem_close(cons);
    sem_unlink(SEM_PROD_FNAME);
    sem_unlink(SEM_CONS_FNAME);
    return 0;
}