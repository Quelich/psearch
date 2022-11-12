/* FIX THIS

- input13.txt keyword is in the 2th line but it prints as 3rd 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

    char *searchKeyword = argv[1]; /* DO NOT CHANGE: reserved for the keyword*/
    int *filesCount = argv[2];     /* DO NOT CHANGE: reserved for the files count*/
    char *outputFileName = malloc(1024);

    // create output file directory
    strcat(outputFileName, ROOT_DIR);
    strcat(outputFileName, argv[argc - 1]);
    //printf("output file: %s\n", argv[argc - 1]);

    for (int a = 3; a < argc - 1; a++)
    {

        FILE *inputFileStream; /* single stream for a file */
        char *fileDir = argv[a];

        char currentFile[WORD_BUFFER];
        if (currentFile == NULL)
        {
            printf("No memory\n");
            return 0;
        }

        // Make a complete dir for the argv[i]
        strcat(currentFile, ROOT_DIR);
        strcat(currentFile, fileDir);

        // Open file content
        if ((inputFileStream = fopen(currentFile, "r")) == NULL)
        {
            printf("Error opening file!\n");
            return 0;
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
        char word[WORD_BUFFER];
        int wordChCount = 0;


        while (((myChar = fgetc(inputFileStream)) != EOF))
        {
            if (myChar == '\n')
            {
                lineCount++;
            }
            // Before beginning a new word
            if (myChar == ' ' || myChar == '\n' || myChar == '\0' || myChar == '\t')
            {

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
        char line[LINE_BUFFER];

        /* DEBUGGING */
        // Print matched line indices
        // for (int i = 0; i < MAX_MATCHED_LINES; i++)
        // {
        //     if (i % 30 == 0)
        //     {
        //         printf("\n");
        //     }
        //     printf("%d ", matchedLinesIndices[i]);
        // }

        //printf("\n");

        // Write all lines to an array
        rewind(inputFileStream);
        int j = 1;
        while ((fgets(line, LINE_BUFFER, inputFileStream)) != NULL)
        {
            strcpy(matchedLines[j], line);
            j++;
        }

        // End of reading file 
        //printf("\n");
        rewind(inputFileStream);
        fclose(inputFileStream);
        


        // Write to output file
        FILE * outputFileStream;

        if ((outputFileStream = fopen(outputFileName, "a")) == NULL)
        {
            printf("Error opening output file!\n");
            return 0;
        }

        int l = 0;
        for (int k = 0; k < MAX_MATCHED_LINES; k++)
        {   
            

            // TODO: write to a file
            if (k == matchedLinesIndices[l])
            {
                fprintf(outputFileStream, "%s, %d: %s\n", currentFile, k, matchedLines[k]);
                l++;
            }
        }
        fclose(outputFileStream);
        *currentFile = '\0';
    }

    return 0;
}