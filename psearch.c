#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROOT_DIR "./"
#define BUFFER_SIZE 209715200 // 200 MB
#define WORD_BUFFER 1024
#define LINE_BUFFER 1024
#define MAX_MATCHED_LINES 1024
#define FOUND 1
#define NOT_FOUND 0

int main(int argc, int **argv)
{

    if (argc < 1)
    {
        printf("Number of input files must be greater than 1!\n");
        return 0;
    }

    char *searchKeyword = argv[1];
    int keywordLen = strlen(searchKeyword);

    /*
    argv[0]: spared for command
    argv[1]: spared for keyword
    ...
    argv[n]: spared for output file
    */

    char *fileBuffer = malloc(BUFFER_SIZE * sizeof(char));          /* the file content*/
    char *currentFile = malloc(sizeof(argv[1]) + sizeof(ROOT_DIR)); /* memory alloc for a file*/

    for (int i = 2; i < argc - 1; i++)
    {
        FILE *fileStream; /* single stream for a file */

        char *fileDir = argv[i];

        if (currentFile == NULL)
        {
            printf("No memory\n");
            return 0;
        }

        // Make a complete dir for the argv[ie]
        strcat(currentFile, ROOT_DIR);
        strcat(currentFile, fileDir);
        // Open file content

        if ((fileStream = fopen(currentFile, "r")) == NULL)
        {
            printf("Error opening file!\n");
            return 0;
        }

        int bufferCount = 0;
        char strBuffer[WORD_BUFFER];

        // Create matched lines indices array
        int matchedLinesIndices[MAX_MATCHED_LINES];
        for (int i = 0; i < MAX_MATCHED_LINES; i++)
        {
            matchedLinesIndices[i] = 0;
        }
        
        int matchedLinesCounter = 0;
        int myChar;
        int lineCounter = 1;
       

        // Read file by word
        while ( ((myChar = fgetc(fileStream)) != EOF) &&
                fscanf(fileStream, "%1023s", strBuffer) == 1)
        {   

            if((char)myChar == '\n')
            {
                lineCounter++;
            }

            if(strcmp(strBuffer, searchKeyword) == 0)
            {
                matchedLinesIndices[matchedLinesCounter] = lineCounter;
                matchedLinesCounter++;
            }

            fileBuffer[bufferCount++] = (char)myChar;
        }

        
        char matchedLines[MAX_MATCHED_LINES][WORD_BUFFER];

        matchedLinesCounter = 0;
        char line[LINE_BUFFER];

        // Print matched line indices
        // for (int i = 0; i < MAX_MATCHED_LINES; i++)
        // {
        //     if(i % 30 == 0)
        //     {
        //         printf("\n");
        //     }
        //     printf("%d ", matchedLinesIndices[i]);
        // }

        // Write all lines to an array
        rewind(fileStream);
        int j = 1;
        while ((fgets(line, LINE_BUFFER, fileStream)) != NULL)
        {   
            
            strcpy(matchedLines[j],line); 
            j++;
        }

    
        // Read matched lines from the array
        printf("\nMatched lines:\n");
        int l = 0;
        for (int k = 0; k < MAX_MATCHED_LINES; k++)
        {
           if (k == matchedLinesIndices[l])
           {
                printf("%s, %d: %s\n", currentFile, k, matchedLines[k]);
                l++;
           }
           
        }

        fclose(fileStream);

        // printf("The %s content:\n%s", currentFile, fileBuffer);
        printf("\n");
        // printf("Current File: %s\n", currentFile);
        // printf("Content: \n%s\n", fileBuffer);
        // printf("Number of lines: %d\n", lineCounter);
        // printf("Detected n: %d\n", i);
        printf("***********************************************\n");

        *currentFile = '\0';
    }

    return 0;
}