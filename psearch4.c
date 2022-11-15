/*
    READ FROM SEMAPHORE
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

#define MAX_INPUT_COUNT 100
#define SEM_FNAME "semaphore"
#define SH_FNAME "/dev/null"
#define SHM_BUFFER 4096
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
        perror("error creating shm_key\n");
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(shm_key, sizeof(int), 0644 | IPC_CREAT);

    if (shm_id < 0)
    {
        perror("Error creating shm_id\n");
        exit(-1);
    }

    char *shrd_value = shmat(shm_id, NULL, 0);

    sem_t * sem = sem_open(SEM_FNAME, O_CREAT | O_EXCL, 0644, sem_value);

    if (sem == SEM_FAILED)
    {
        perror("error opening semaphore");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < fork_count; i++)
    {
        char *inputFile = inputFiles[i];
        pid = fork();

        if (pid < 0)
        {
            perror("error creating fork");
            sem_unlink(SEM_FNAME);
            sem_close(sem);
            exit(EXIT_FAILURE);
        }

        else if (pid == 0) /* CHILD PROCESS */
        {
            execlp("./psearch3slave", "psearch3slave", searchKeyword, inputFile, NULL);
            break;
        }
    }

    if (pid == 0) /* CHILD PROCESS */
    {
        sem_wait(sem);
        printf("[CHILD] - %d is in critical section\n", i);

        // char msg[50] = "Hello from";
        //sprintf(shrd_value, "%s #%d", msg, i);
        // printf("[CHILD] - %d: new value of shrd_value = %s\n", i, shrd_value);
        sem_post(sem);
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

        printf("All children exited\n%s", shrd_value);
        shmdt(shrd_value);
        shmctl(shm_id, IPC_RMID, 0);
        sem_unlink(SEM_FNAME);
        sem_close(sem);
        exit(EXIT_SUCCESS);
    }

    return EXIT_SUCCESS;
}