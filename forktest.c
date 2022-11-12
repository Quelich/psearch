#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, int** argv)
{
    fork(); // 2
    fork(); // 4
    printf("Hello WOrld\n");
    // int f_id = fork();
    // if (f_id < 0)
    // {
    //     /* UNABLE TO CREATE CHILD */
    //     printf("Couldn\'t create child process");
    //     return 0;
    // }
    
    // else if(f_id == 0)
    // {
    //     /* CHILD */
    //     printf("Hello World from child:%d\n", f_id);
    // }

    // else
    // {
    //     /* PARENT */
    //      printf("Hello World from parent:%d\n", f_id);
    // }
        
    return 0;
}