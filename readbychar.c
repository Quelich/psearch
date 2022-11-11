
        // // Read file by character
        // while ((myChar = fgetc(fileStream)) != EOF)
        // {
        //     // Count the number of lines
        //     if (myChar == '\n')
        //     {
        //         lineCounter++;
        //     }

        //     // Find the first character of the keyword
        //     if (searchKeyword[0] == (char)myChar)
        //     {

        //         int isFound = 0;
        //         int foundChar;
        //         int foundCharCount = 1;
        //         int n = bufferCount;

        //         // Check the rest of the keyword
        //         while ((foundChar = fgetc(fileStream)) != EOF && (foundCharCount != keywordLen))
        //         {
        //             // printf("Char: %c Buffer: %c\n", searchKeyword[foundCharCount], (char)foundChar);

        //             // GUARD: not found
        //             if (searchKeyword[foundCharCount] != (char)foundChar)
        //             {
        //                 isFound = 0;
        //                 break;
        //             }

        //             // Feed the buffer for the found char scan
        //             fileBuffer[n++] = (char)foundChar;

        //             // Feed the buffer for the file scan
        //             fileBuffer[bufferCount++] = (char)foundChar;

        //             foundCharCount++;
        //             isFound = 1;
        //         }

        //         // TODO: found : print <input_filename>, <matched_line_number>: <matched_line>
        //         if (isFound == FOUND)
        //         {
        //             printf("Found\n");
        //         }

        //         if (isFound == NOT_FOUND)
        //         {
        //             printf("Not Found\n");
        //         }
        //     }

        //     fileBuffer[bufferCount++] = (char)myChar;
        // }