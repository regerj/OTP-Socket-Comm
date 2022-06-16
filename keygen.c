#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

bool isInt(char * str);                     // Function to determine if a c string is an int
void generateKey(int len);                  // Function to generate a key and output to stdout

int main(int argc, char** argv)             // Here argc should be 2 and argv[1] should be a number representing the length of the key
{
   if(argc != 2)                            // If incorrect number of args
    {
                                            // Output the error
        printf("Incorrect number of arguments! use the syntax [keygen keylength].\n");
    }

    if(!isInt(argv[1]))                     // If its not a number
        return 1;                           // Return with code 1

    generateKey(atoi(argv[1]));             // Generate the key
    return 0;                               // Return with code 0
}

bool isInt(char * str)                      // Function to determine if a c string is an int                 
{   
    for(int i = 0; i < strlen(str); i++)
    {
        if(!isdigit(str[i]))
        {
            printf("Argument passed in is not an integer! Please try again.\n");
            return false;
        }
    }
    return true;
}

void generateKey(int len)
{
    for(int i = 0; i < len; i++)
    {
        int currChar = 0;
        currChar = (rand() % 27) + 65;
        if(currChar == 91)
            currChar = 32;
        printf("%c", currChar);
    }
    printf("\n");
    return;
}