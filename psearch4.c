/*
    READ FROM SEMAPHORE
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <memory.h>
#include <fcntl.h>
#include <semaphore.h>

#define MAX_INPUT_COUNT 100
#define CHILD_EXEC_DIR "./psearch4slave"
#define CHILD_EXEC_CMD "psearch4slave"
#define SEM_PRODUCER_FNAME "/producer"
#define SEM_CONSUMER_FNAME "/consumer"
#define SHM_FNAME "/shdmem"
#define SHM_BUFFER 4096
#define SPH_BUFFER 1024

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
    int fileCount = 0;
    for (int d = 2; d < argc - 1; d++)
    {
        inputFiles[fileCount] = argv[d];
        fileCount++;
    } /* end -- RESERVED */

    sem_t *sem_prod = sem_open(SEM_PRODUCER_FNAME, 0);
    sem_t *sem_cons = sem_open(SEM_CONSUMER_FNAME, 1);
    int fd = shm_open(SHM_FNAME, O_RDWR, (mode_t)0777);

    if (sem_prod == SEM_FAILED)
    {
        perror("[MASTER] sem_open/producer");
        exit(-1);
    }

    if (sem_cons == SEM_FAILED)
    {
        perror("[MASTER] sem_open/consumer");
        exit(-1);
    }

    if (fd < 0)
    {
        perror("[MASTER]Error opening file descriptor!\n");
        exit(-1);
    }

    /* MAKE & EXECUTE CHILD PROCESSES */
    for (int i = 0; i < fileCount; i++)
    {
        char *inputFile = inputFiles[i];
        int p_id = fork();

        if (p_id < 0)
        {
            perror("[MASTER] Error creating child process!\n");
            exit(-1);
        }

        else if (p_id == 0) /* CHILD PROCESS */
        {
            execlp(CHILD_EXEC_DIR, CHILD_EXEC_CMD, searchKeyword, inputFile, NULL);
            exit(0);
        }
    }

    for (int i = 0; i < fileCount; i++)
    {
        wait(NULL);
    }

    char *shdmem = (char *)mmap(0, SHM_BUFFER,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED, fd, 0);

    if (shdmem == MAP_FAILED)
    {
        close(fd);
        perror("[MASTER] Error opening shared memory!\n");
        exit(-1);
    }

    char final_msg[SHM_BUFFER] = {0x0};
    // printf("[MASTER] listerning slaves\n");
    /* WRITE TO THE SHARED MEMORY */
    while (true)
    {
        sem_wait(sem_prod);
        char msg[SPH_BUFFER] = {0x0};
        strcpy(msg, shdmem);
        printf("[MASTER] MESSAGE RECEIVED:\n%s\n", msg);
        strcat(final_msg, msg);

        /* WRITE TO THE OUTPUT FILE */
        FILE *outputStream;
        if ((outputStream = fopen(outputFileName, "w")) == NULL)
        {
            perror("[MASTER] Error opening output file!\n");
            exit(-1);
        }

        fprintf(outputStream, "%s", final_msg);

        // shdmem[0] = 0; /* RESET SHARED MEMORY */
        sem_post(sem_cons);
    }

    sem_close(sem_cons);
    sem_close(sem_prod);

    if (munmap(shdmem, strlen(shdmem)) == -1)
    {
        perror("[MASTER] Error freeing shared memory!\n");
        exit(-1);
    }

    return 0;
}