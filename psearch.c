/* FIX THIS

- when the keyword at the start it cannot be found

*/

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

   
    char *searchKeyword = argv[1]; /* DO NOT CHANGE: reserved for the keyword*/
    int *filesCount = argv[2]; /* DO NOT CHANGE: reserved for the files count*/

    for (int a = 3; a < argc - 1; a++)
    {
       
        FILE *fileStream; /* single stream for a file */
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
        if ((fileStream = fopen(currentFile, "r")) == NULL)
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
        int lineCount = 1;
        char strBuffer[WORD_BUFFER];

        // Read file by word and character
        // while (((myChar = fgetc(fileStream)) != EOF) &&
        //        fscanf(fileStream, "%1023s", strBuffer) == 1)
        // {

        //     if ((char)myChar == '\n')
        //     {
        //         lineCount++;
        //     }

        //     if (strcmp(strBuffer, searchKeyword) == 0)
        //     {
        //         matchedLinesIndices[matchedLinesCount] = lineCount;
        //         matchedLinesCount++;
        //     }
        // }

        char word[WORD_BUFFER];
        int wordChCount = 0;
        while (((myChar = fgetc(fileStream)) != EOF))
        {
            
            if ((char)myChar == '\n')
            {
                lineCount++;
            }

            
            // Before beginning a new word
            if (myChar == ' ' || myChar == '\n' || myChar == '\0' || myChar == '\t'  )
            {   
               if (strcmp(word, searchKeyword) == 0)
               {
                    matchedLinesIndices[matchedLinesCount] = lineCount;
                    matchedLinesCount++;
               }
                
               // Reset word 
               word[0] = 0;
               wordChCount = 0;
            }
            else
            {
                word[wordChCount++] = (char)myChar;
                word[wordChCount] = 0;

            }
        }
        

        char matchedLines[MAX_MATCHED_LINES][WORD_BUFFER];
        char line[LINE_BUFFER];

        /* DEBUGGING */
        // Print matched line indices
        for (int i = 0; i < MAX_MATCHED_LINES; i++)
        {
            if(i % 30 == 0)
            {
                printf("\n");
            }
            printf("%d ", matchedLinesIndices[i]);
        }

        printf("\n");
        
        // Write all lines to an array
        rewind(fileStream);
        int j = 1;
        while ((fgets(line, LINE_BUFFER, fileStream)) != NULL)
        {
            strcpy(matchedLines[j], line);
            j++;
        }

        // Read matched lines from the array
        int l = 0;
        for (int k = 0; k < MAX_MATCHED_LINES; k++)
        {
            if (k == matchedLinesIndices[l])
            {
                printf("%s, %d: %s\n", currentFile, k, matchedLines[k]);
                l++;
            }
        }

        printf("\n");
        rewind(fileStream);
        fclose(fileStream);
        *currentFile = '\0';
    }

    return 0;
}