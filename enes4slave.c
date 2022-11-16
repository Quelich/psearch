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
#include <semaphore.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* Global variables */
char lines_of_file[1024];
FILE *fp;
FILE *temp_output_file;
FILE *merged_output_file;
size_t count_line = 0;
int process_status;
char *sharedfile_path = "./shared_file.txt";

int main(int argc, char *argv[])
{
    sem_unlink("master");
    sem_unlink("slave");

    char *search_keyword = argv[1];
    char *input_file = argv[2];

    count_line = 0; // to store the line where we find the search keyword

    char line_output[3092] = {0x0};   // to store matched line
    char merged_output[4096] = {0x0}; // to store the all matched lines

    fp = fopen(input_file, "r");                                    // opening input files to read
    while (fgets(lines_of_file, sizeof(lines_of_file), fp) != NULL) // read throguh the file and store each line in lines_of_file
    {
        char *temp_lines = lines_of_file;
        count_line++; // increment counter in each line
        if ((temp_lines = strstr(lines_of_file, search_keyword)) != NULL)
        {
            char *space = temp_lines + strlen(search_keyword);
            if (temp_lines == lines_of_file || isblank((unsigned char)*(temp_lines - 1)))
            {
                /* if the exact keyword found in a line */
                if (*space == '\0' || isblank((unsigned char)*space))
                {
                    // printf("\n%s, %d: %s", input_file, (int)count_line, lines_of_file);

                    sprintf(line_output, "%s, %d: %s", input_file, (int)count_line, lines_of_file); // store the matched line in a buffer
                    strcat(merged_output, line_output);                                             // merge the matched lines
                }
            }
            temp_lines = space;
        }
    }
    strcat(merged_output, "\n"); // add new line after a file read

    /**/
    sem_t *prod = sem_open("master", O_CREAT | O_EXCL, 0644, 0); // open a named semaphore for master
    sem_t *cons = sem_open("slave", O_CREAT | O_EXCL, 0644, 1);  // open a named semaphore for slave

    if (prod == SEM_FAILED)
    {
        perror("[SLAVE]\nsemaphore/producer");
        exit(EXIT_FAILURE);
    }

    if (cons == SEM_FAILED)
    {
        perror("[MASTER]\nsemaphore/consumer");
        exit(EXIT_FAILURE);
    }

    /*
    key_t shm_key = ftok("/dev/null", 0);

    int id = shmget(shm_key, sizeof(int), 0644 | IPC_CREAT); // return the shared memory identifier with a key
    char *mem = (char *)shmat(id, ( void *) 0, 0);
    if (id == -1){
      printf ("Error in creating! ERRNO: %d\n", errno);
    }
    else{
      printf("Created: %d\n" , id);
    }
    */
    struct stat f_stat;
    size_t len = 4096;

    int fd = shm_open("slave", O_CREAT | O_RDWR, 0666);

    char *mmap_address = (char *)mmap(0, 10000, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, fd, 0);

    

    if (ftruncate(fd, len) == -1)
    { 
        perror("ftruncate");
        return 1;
    }

    /* if mapping fails */
    if (mmap_address == MAP_FAILED)
    {
        printf("Error! Failed to open shared memory! ERRNO: %d\n", errno);
        exit(0);
    }

    sem_wait(cons);
    strcat(mmap_address, merged_output); // passing output to shared memory segment
    printf("Memory segment: %s\n", mmap_address);
    sem_post(prod); // sginal the producer semaphoreS

    // shmdt(mmap_address); // detach
    sem_close(prod);
    sem_close(cons);
   
    return 0;
}