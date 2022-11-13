#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, int **argv)
{

    if (argc < 1)
    {
        printf("Number of input files must be greater than 1!\n");
        return 0;
    }

    char *searchKeyword = argv[1];
    char *inputFile = argv[2];
    
    return 0;
}