#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/mman.h>
#include <ctype.h>
#include <unistd.h>
#include <memory.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* function prototypes */
void psearch4(char *keyword, int num_of_input_files, char *input_file_names[], char *output_file_name);
static int comparator(const void *a, const void *b);

/* global variables */
int num_of_files;
const char *sf_path = "./shared_file.txt";
FILE *merged_final_file;

int main(int argc, char *argv[])
{

    char *search_key = argv[1];         // store search keyeword
    char *input_files[255];             // store input file names
    char *output_file = argv[argc - 1]; // store output file name

    for (int i = 2; i < argc - 1; i++)
    {
        num_of_files++; // each loop increment num of files
    }

    /* storing input file names */
    for (int i = 0; i < num_of_files; i++)
    {
        input_files[i] = argv[i + 2];
        // printf("%s\n", input_files[i]);
    }
    // printf("%s\n", search_key);
    // printf("%d\n", num_of_files);

    /* sort files according to their names*/
    qsort(input_files, num_of_files, sizeof(const char *), comparator);

    /* call the function */
    psearch4(search_key, num_of_files, input_files, output_file);
}

/*To compare a and b*/
static int comparator(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

void psearch4(char *keyword, int num_of_input_files, char *input_file_names[], char *output_file_name)
{
    struct stat f_stat;
    size_t len = 4096;

    int fd = shm_open("slave", O_CREAT | O_RDWR, 0666);
    char *mmap_address = (char *)mmap(0, 10000, PROT_READ | PROT_WRITE, MAP_SHARED , fd, 0);
    
   
    if (ftruncate(fd, len) == -1)
    {
        perror("ftruncate");
    }
    sem_unlink("master");
    sem_unlink("slave");
    sem_t *master = sem_open("master", O_CREAT | O_EXCL, 0644, 0);
    sem_t *slave = sem_open("slave", O_CREAT | O_EXCL, 0644, 1);

    if (master == SEM_FAILED)
    {
        perror("[SLAVE]\nsemaphore/producer");
        exit(EXIT_FAILURE);
    }

    if (slave == SEM_FAILED)
    {
        perror("[MASTER]\nsemaphore/consumer");
        exit(EXIT_FAILURE);
    }

    pid_t pid;

    /* iterate through all files*/
    for (int i = 0; i < num_of_input_files; ++i)
    {
        /* fork a chlid*/
        pid = fork();

        /* if fork fails*/
        if (pid < 0)
        {
            sem_close(master);
            sem_close(slave);
            sem_unlink("master");
            sem_unlink("slave");
            fprintf(stderr, "Fork Failed!");
            exit(0);
        }
        /* CHILD PROCESS */
        else if (pid == 0)
        {
            /* execute slave process */
            execl("enes4slave", "enes4slave", keyword, input_file_names[i], NULL);
            exit(0);
        }

        wait(NULL);
    }
    /* PARENT PROCESS*/

    if (pid == 0)
    {

        while (true)
        {
            sem_wait(master);

            if (strlen(mmap_address) > 0)
            {

                break;
            }

            sem_post(slave);
        }

        exit(1);
    }

    else if (pid > 0)
    {

        FILE *final_output_file = fopen(output_file_name, "w");
        printf("MASTER MAP: \n%s\n", mmap_address);
        fprintf(final_output_file, "%s", mmap_address);
        shmdt(mmap_address);
        sem_close(master);
        sem_close(slave);
        fclose(final_output_file);
    }
}