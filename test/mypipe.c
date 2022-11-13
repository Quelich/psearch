#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER 50

int main(int argc, int** argv)
{   
    /*
        fd[0] - read
        fd[1] - write
    */
    int fd[2];

    if (pipe(fd) < 0)
    {
        perror("Error opening pipe!\n");
        return 1;
    }

    const char msg[BUFFER] = "my pipe is big";

    int p_id = fork();

    if(p_id < 0)
    {
        perror("Error opening fork!\n");
        return 1;
    }

    else if(p_id == 0) /* CHILD PROCESS */
    {   
        close(fd[0]);
        write(fd[1],msg, sizeof(msg));
        close(fd[1]); /* END WRITING TO PIPE */
    }

    else  /* PARENT PROCESS */
    {   
         close(fd[1]); /* END WRITING TO PIPE */
        char read_msg[BUFFER];
        if (read(fd[0], read_msg, sizeof(read_msg)) == -1)
        {
            perror("Error reading pipe message!\n");
            return 1;
        }
        
        printf("Got from child process: %s\n", read_msg);
    }
    
    
}