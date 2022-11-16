/*
    READ FROM SEMAPHORE
*/

#include <stdio.h>
#include <stdbool.h>
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

#define MAX_INPUT_COUNT 50
#define SEM_PROD_FNAME "producer"
#define SEM_CONS_FNAME "consumer"
#define SH_FNAME "/dev/null"
#define SHM_BUFFER 4096
#define MSG_BUFFER 1024
#define TOTAL_MSG_BUFFER 10240
#define SPH_BUFFER 512

int main(int argc, int **argv)
{

    if (argc < 1)
    {
        perror("[MASTER] Number of input files must be greater than 1!\n");
        return 0;
    }

    /* RESERVED */
    char *searchKeyword = argv[1];
    char *inputFiles[MAX_INPUT_COUNT];
    char *outputFileName = argv[argc - 1];
    int fileCount = 0; /* also fork count */
    for (int d = 2; d < argc - 1; d++)
    {
        inputFiles[fileCount] = argv[d];
        fileCount++;
    }

    int i;
    pid_t pid;
    key_t shm_key;
    int shm_id;
    int fork_count = fileCount;
    unsigned int sem_value;

    shm_key = ftok(SH_FNAME, 0);

    if (shm_key == -1)
    {
        perror("[MASTER]\nerror creating shm_key\n");
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(shm_key, MSG_BUFFER, 0644 | IPC_CREAT);

    if (shm_id < 0)
    {
        perror("Error creating shm_id\n");
        exit(-1);
    }

    /* ATTACH MEMORY */
    char *memblock = shmat(shm_id, NULL, 0);

    sem_unlink(SEM_PROD_FNAME);
    sem_unlink(SEM_CONS_FNAME);

    sem_t *prod = sem_open(SEM_PROD_FNAME, O_CREAT | O_EXCL, 0644, 0);

    if (prod == SEM_FAILED)
    {
        perror("[MASTER]\nsemaphore/producer");
        exit(EXIT_FAILURE);
    }

    sem_t *cons = sem_open(SEM_CONS_FNAME, O_CREAT | O_EXCL, 0644, 1);

    if (cons == SEM_FAILED)
    {
        perror("[MASTER]\nsemaphore/consumer");
        exit(EXIT_FAILURE);
    }

    /* CREATE CHILD PROCESSES */ 
    for (i = 0; i < fork_count; i++)
    {
        char *inputFile = inputFiles[i];
        pid = fork();

        if (pid < 0)
        {
            perror("error creating fork");
            sem_unlink(SEM_PROD_FNAME);
            sem_close(prod);
            sem_unlink(SEM_CONS_FNAME);
            sem_close(cons);
            exit(EXIT_FAILURE);
        }

        else if (pid == 0) /* CHILD PROCESS */
        {
            execlp("./psearch4slave", "psearch4slave", searchKeyword, inputFile, NULL);
            break;
        }
    }

    char total_msg[TOTAL_MSG_BUFFER];

    if (pid == 0) /* CHILD PROCESS */
    {
        while (true)
        {
            sem_wait(prod);
            if (strlen(memblock) > 0)
            {
              
                /* READING CHILD PROCESSES */
                char child_msg[MSG_BUFFER] = {0x0};
                sprintf(child_msg, "%s", memblock);
                strcat(total_msg, child_msg);
                memblock[0] = 0; /* RESET SHARED MEMORY */
                break;
            }
            sem_post(cons);
        }

        exit(EXIT_SUCCESS);
    }

    else if (pid > 0) /* PARENT PROCESS */
    {

        /* WAIT CHILD PROCESSES TO FINISH */
        while (pid = waitpid(-1, NULL, 0))
        {

            if (errno == ECHILD)
            {
                break;
            }
        }

        printf("[MASTER]\nTotal Message:\n%s", memblock);

        /* WRITE TO THE OUTPUT FILE */
        FILE *outputStream;
        if ((outputStream = fopen(outputFileName, "w")) == NULL)
        {
            perror("[MASTER] Error opening output file!\n");
            exit(-1);
        }

        fprintf(outputStream, "%s", memblock);
        fclose(outputStream);

        shmdt(memblock);
        shmctl(shm_id, IPC_RMID, 0);
        // sem_unlink(SEM_PROD_FNAME);
        // sem_unlink(SEM_CONS_FNAME);
        sem_close(prod);
        sem_close(cons);
        exit(EXIT_SUCCESS);
    }

    return EXIT_SUCCESS;
}