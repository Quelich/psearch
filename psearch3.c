#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <memory.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_INPUT_COUNT 10
#define SHD_FNAME "./shared_output.txt"

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
    int fileCount = 0;
    for (int d = 2; d < argc - 1; d++)
    {
        inputFiles[fileCount] = argv[d];
        fileCount++;
    }
    /* RESERVED */

    /* MAKE CHILD PROCESSES */
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
            execlp("./psearch3slave", "psearch3slave", searchKeyword, inputFile, NULL);
            exit(0);
        }

        wait(NULL);
    }


    int fd = open(SHD_FNAME, O_CREAT | O_RDWR, (mode_t)0777);

    if (fd < 0)
    {
        perror("[MASTER]Error opening file descriptor!\n");
        exit(-1);
    }

    struct stat fstatus;
    fstat(fd, &fstatus);
    off_t fstatus_s = fstatus.st_size;

    char *shdmem = (char *)mmap(0, fstatus_s, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_SHARED, fd, 0);

    if (shdmem == MAP_FAILED)
    {
        close(fd);
        perror("[MASTER] Error opening shared memory!\n");
        exit(-1);
    }

    /* WRITE TO THE OUTPUT FILE */
    FILE *outputStream;
    if ((outputStream = fopen(outputFileName, "w")) == NULL)
    {
        perror("[MASTER] Error opening output file!\n");
        exit(-1);
    }

    fprintf(outputStream, "%s", shdmem);

    /* DEALLOCATE SHARED MEMORY */
    if (munmap(shdmem, strlen(shdmem)) == -1)
    {
        perror("[MASTER] Error freeing shared memory!\n");
        exit(-1);
    }

    close(fd);
    remove(SHD_FNAME);

    /* STOP TIME MEASUREMENT */
    clock_gettime(CLOCK_REALTIME, &end);
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    double elapsed_time = seconds + nanoseconds * 1e-9;

    printf("Time Measured in seconds: %f\n", elapsed_time);
    printf("Time Measured in nanoseconds: %ld\n", nanoseconds);

    return 0;
}