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
#include <time.h>

#define MAX_INPUT_COUNT 10
#define SEM_PROD_FNAME "producer"
#define SEM_CONS_FNAME "consumer"
#define FD_FNAME "semaphore"
#define SH_FNAME "/dev/null"
#define SHM_BUFFER 102400 /* 10 MB */
#define MSG_BUFFER 4096
#define TOTAL_MSG_BUFFER 1024000
#define SPH_BUFFER 4096

char total_msg[TOTAL_MSG_BUFFER];

int main(int argc, int **argv)
{
    /* START TIME MEASUREMENT */
    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);

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
    int fork_count = fileCount;

    // unsigned int sem_value;
    // key_t shm_key;
    // int shm_id;

    // shm_key = ftok(SH_FNAME, 0);

    // if (shm_key == -1)
    // {
    //     perror("[MASTER]\nerror creating shm_key\n");
    //     exit(EXIT_FAILURE);
    // }

    // shm_id = shmget(shm_key, MSG_BUFFER, 0644 | IPC_CREAT);

    // if (shm_id < 0)
    // {
    //     perror("Error creating shm_id\n");
    //     exit(-1);
    // }

    /* ATTACH MEMORY */
    // char *memblock = shmat(shm_id, NULL, 0);
    int fd = shm_open(FD_FNAME, O_CREAT | O_RDWR, 0666);

    if (ftruncate(fd, SHM_BUFFER) == -1)
    {
        perror("ftruncate");
    }

    char *memblock = (char *)mmap(0, SHM_BUFFER, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_PRIVATE, fd, 0);

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
            exit(EXIT_SUCCESS);
        }
        wait(NULL);
    }

    if (pid == 0) /* CHILD */
    {
        while (true)
        {
            sem_wait(prod);
            if (strlen(memblock) > 0)
            {
                printf("[MASTER]\nReading:\n%s", memblock);
                strcat(total_msg, memblock);
                memblock[0] = 0;
                break;
            }

            sem_post(cons);
        }
        shmdt(memblock);
        exit(EXIT_SUCCESS);
    }

    else if (pid > 0) /* PARENT PROCESS */
    {

        while (pid = waitpid(-1, NULL, 0))
        {

            if (errno == ECHILD)
            {
                break;
            }
        }

        /* WRITE TO THE OUTPUT FILE */
        FILE *outputStream;
        if ((outputStream = fopen(outputFileName, "w")) == NULL)
        {
            perror("[MASTER] Error opening output file!\n");
            exit(-1);
        }
        //printf("[MASTER]\nTotal Message:\n%s\n", memblock);
        fprintf(outputStream, "%s", memblock);
        fclose(outputStream);

        shmdt(memblock);
        // shmctl(shm_id, IPC_RMID, 0);
        sem_unlink(SEM_PROD_FNAME);
        sem_unlink(SEM_CONS_FNAME);
        sem_close(prod);
        sem_close(cons);

        /* STOP TIME MEASUREMENT */
        clock_gettime(CLOCK_REALTIME, &end);
        long seconds = end.tv_sec - begin.tv_sec;
        long nanoseconds = end.tv_nsec - begin.tv_nsec;
        double elapsed_time = seconds + nanoseconds * 1e-9;

        printf("Time Measured in seconds: %f\n", elapsed_time);
        printf("Time Measured in nanoseconds: %ld\n", nanoseconds);

        exit(EXIT_SUCCESS);
    }
    return EXIT_SUCCESS;
}