#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>
#include <fcntl.h>

#define OUTPUT_BUFFER_SIZE 1024 /* 1 MB */

int main(int argc, int **argv)
{

    if (argc < 1)
    {
        printf("Number of input files must be greater than 1!\n");
        return 0;
    }

    char *smem = (char *)mmap(NULL, OUTPUT_BUFFER_SIZE,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    if (smem == MAP_FAILED)
    {
        perror("Error creating shared memory!\n");
        return 1;
    }

    /* DO NOT CHANGE: RESERVED */
    char *searchKeyword = argv[1];
    char *inputFiles[100];
    char *outputFileName = argv[argc - 1];
    int e = 0;
    for (int d = 2; d < argc - 1; d++)
    {
        inputFiles[e] = argv[d];
        e++;
    }

    char msg[OUTPUT_BUFFER_SIZE] =  "shake it off";


    int p_id = fork();
    
    if (p_id < 0)
    {
        printf("Error creating child process!\n");
        exit(-1);
    }

    else if (p_id == 0) /* CHILD PROCESS */
    {
        smem = "mr robot";
    }

    else /* PARENT PROCESS */
    {
        wait(NULL);
    }

    printf("shared msg: %s\n", smem);

    // if (munmap(smem, OUTPUT_BUFFER_SIZE) == -1)
    // {
    //     //remove file
    //     perror("Error freeing shared memory!");
    //     exit(-1);
    // }

    return 0;
}