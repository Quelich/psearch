#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/mman.h>
#include <fcntl.h>
#include <memory.h>

#define ROOT_DIR "./"
#define WORD_BUFFER 100
#define LINE_BUFFER 4096
#define MAX_MATCHED_LINES 2048
#define SMEM_BUFFER 10240
#define SHD_FNAME "./shared_output.txt"

int main(int argc, int **argv)
{
    if (argc < 1)
    {
        printf("[SLAVE] Number of input files must be greater than 1!\n");
        return 0;
    }

    char *searchKeyword = argv[1];
    char *inputFile = argv[2];
    char currentInputFileDir[100];

    // Make a complete directory for the input file
    sprintf(currentInputFileDir, "%s%s", ROOT_DIR, inputFile);

    // printf("Current file dir: %s\n", currentInputFileDir);
    if (currentInputFileDir == NULL)
    {
        perror("[SLAVE] Current file directory is NULL!\n");
        return 1;
    }

    FILE *inputFileStream; /* single stream for a file */

    // Open file content
    if ((inputFileStream = fopen(currentInputFileDir, "r")) == NULL)
    {
        perror("[SLAVE] Error opening input file!\n");
        exit(-1);
    }

    // Create matched lines indices array
    int matchedLinesIndices[MAX_MATCHED_LINES];
    for (int b = 0; b < MAX_MATCHED_LINES; b++)
    {
        matchedLinesIndices[b] = 0;
    }

    int matchedLinesCount = 0;
    int myChar;
    int lineCount = 0;
    char word[WORD_BUFFER] = {0x0};
    int wordChCount = 0;

    // Read file by character to find matched line indices
    while (((myChar = fgetc(inputFileStream)) != EOF))
    {
        if (myChar == '\n')
        {
            lineCount++;
        }

        // Before beginning a new word
        if (myChar == ' ' || myChar == '\n' || myChar == '\0' || myChar == '\t')
        {
            // Is it the keyword?
            if (strcmp(word, searchKeyword) == 0)
            {
                matchedLinesIndices[matchedLinesCount] = lineCount + 1;
                matchedLinesCount++;
            }

            // Reset word
            word[0] = '\0';
            wordChCount = 0;
        }
        else
        {
            word[wordChCount++] = (char)myChar;
            word[wordChCount] = '\0';
        }
    }

    char matchedLines[MAX_MATCHED_LINES][WORD_BUFFER];
    char line[LINE_BUFFER] = {0x0};

    // Reset input stream
    rewind(inputFileStream);

    // GET lines by the matched lines indices
    int j = 1;
    while ((fgets(line, LINE_BUFFER, inputFileStream)) != NULL)
    {
        strcpy(matchedLines[j], line);
        j++;
    }

    fclose(inputFileStream);

    /*
          COMPOSE A MESSAGE TO SEND SHARED MEMORY
          format: <input_file>, <matched_line_index>: <matched_line>
    */
    int l = 0;
    char msg[SMEM_BUFFER] = {0x0};

    // printf("Message sent to SHARED MEMORY:\n");
  
    for (int k = 0; k < MAX_MATCHED_LINES; k++)
    {
        if (k == matchedLinesIndices[l])
        {
            char line[LINE_BUFFER] = {0x0};
            sprintf(line, "%s, %d: %s", inputFile, k, matchedLines[k]);
            strcat(msg, line);
            // printf("%s\n", msg); 
            l++;
        }
    }

    
    int fd = open(SHD_FNAME, O_CREAT | O_RDWR, (mode_t)0777);

    if (fd < 0)
    {
        perror("[MASTER]Error opening file descriptor!\n");
        exit(-1);
    }
    //sleep(1);
    struct stat fstatus;
    fstat(fd, &fstatus);
    off_t fstatus_s = fstatus.st_size;

    fallocate(fd, 0, fstatus_s, strlen(msg));

    char *shdmem = (char *)mmap(0, fstatus_s + strlen(msg), PROT_READ | PROT_WRITE,
                               MAP_SHARED, fd, 0);

    if (shdmem == MAP_FAILED)
    {
        close(fd);
        perror("[SLAVE] Error opening shared memory!\n");
        exit(-1);
    }

    for (int i = 0; i < strlen(msg); i++)
    {
       shdmem[fstatus_s + i] = msg[i];
    }

    if (munmap(shdmem, strlen(shdmem)) == -1)
    {
        close(fd);
        perror("[SLAVE] Error freeing shared memory!\n");
        exit(-1);
    }

    close(fd);

    return 0;
}