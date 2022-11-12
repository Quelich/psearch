#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int global_counter = 0;
int main(int argc, int **argv)
{

    // fork(); // 2
    // fork(); // 4
    // for (int i = 0; i < 3; i++)
    // {
    //     if (fork() == 0)
    //     {
    //         /* CHILD */
    //         printf("[child] pid: %d [parent] pid: %d\n", getpid(), getppid());
    //         exit(0);
    //     }
    // }

    // for(int j = 0; j < 3; j++)
    // {
    //     wait(NULL);
    // }

    for (int i = 0; i < 4; i++)
    {
        int f_id = fork();
        

        if (f_id < 0)
        {
            /* UNABLE TO CREATE CHILD */
            printf("Couldn\'t create child process");
            return 0;
        }

        else if (f_id == 0)
        {
            /* CHILD */
            // char *my_args[] = {
            //     "m",
            //     "myarg1",
            //     "myarg2",
            //     NULL};

            // execvp("./forktest", my_args);
            // for (int i = 0; i < argc; i++)
            // {
            //     printf("%s\n", argv[i]);
            // }
           
            exit(0);
        }

        else
        {
            /* PARENT */
            
            printf("Counter: %d\n", global_counter++);
        }
    }

    for (int i = 0; i<4; i++)
    {
      wait(NULL);
    }
    

    // char *files[1024];
    // int j = 0;
    // for (int i = 3; i < argc - 1; i++)
    // {
    //     files[j] = argv[i];
    //     j++;
    // }

    return 0;
}