#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char * msg)
{
    perror(msg);
    exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in * address, int portNumber, char * hostName)
{
    memset((char *) address, '\0', sizeof(*address));           // Clear the address struct
    address->sin_family = AF_INET;                              // Set it to be network capable
    address->sin_port = htons(portNumber);                      // Store the port number

    struct hostent* hostInfo = gethostbyname(hostName);         // Get the DNS entry for the host by its name
    if(hostInfo == NULL)                                        // If failed to retrieve
    {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");       // Output error
        fflush( stderr );
        exit(0);                                                // Exit with code 0
    }

    // Copy the host address into the sockaddr_in struct
    memcpy((char *) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

int main(int argc, char * argv[])
{
    int socketHandle, portNumber, charsWritten, charsRead;      // Declare ints
    struct sockaddr_in serverAddress;                           // Declare the socket info

    portNumber = atoi(argv[3]);

    if(argc < 4)                                                // If cmd line args are invalid
    {
        fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]); // Output error
        fflush( stderr );
        exit(0);                                                // Exit with code 0
    }

    socketHandle = socket(AF_INET, SOCK_STREAM, 0);             // Instantiate a socket over network
    if(socketHandle < 0)                                        // If failed instantiation
    {
        error("CLIENT: ERROR opening socket");                  // Output error
    }

    setupAddressStruct(&serverAddress, portNumber, "localhost");// Setup the address struct using the port number and host name

    // TODO Make input come from text file

    FILE * plainText = NULL;                                    // Handle to file for plaintext
    plainText = fopen(argv[1], "r");                            // Open the file for reading
    if(plainText == NULL)                                       // If the file didnt open
    {
        error("CLIENT: ERROR reading plaintext file, DNE");     // Error outputs
    }

    char str[70000];                                            // String to store msg
    fgets(str, sizeof(str), plainText);                         // Grab msg

    str[strcspn(str, "\n")] = '\0';                             // Strip newline

    fclose(plainText);                                          // Close file

    FILE * keyHandle = NULL;                                    // Key
    keyHandle = fopen(argv[2], "r");                            // Open it
    if(keyHandle == NULL)                                       // If it didnt open
    {
        error("CLIENT: ERROR reading keyHandle file, DNE");     // Error output
    }

    char key[80000];                                            // String for key          
    fgets(key, sizeof(key), keyHandle);                         // Grab key

    key[strcspn(key, "\n")] = '\0';                             // Strip newline

    fclose(keyHandle);                                          // Close the file

    char toTransmit[150000] = { '\0' };                         // Clear a new string
    strcpy(toTransmit, str);                                    // Put in msg
    strcat(toTransmit, "!");                                    // Put in delimiter
    strcat(toTransmit, key);                                    // Put in key
    strcat(toTransmit, "!");                                    // Put in delimiter
    strcat(toTransmit, "D");                                    // Put in ID
    strcat(toTransmit, "!");                                    // Put in delimiter

    // Attempt to connect to the server, if it fails
    if(connect(socketHandle, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        error("CLIENT: ERROR connecting");                      // Output error message
    }

    // Attempt to transmit the buffer over the socket
    charsWritten = send(socketHandle, toTransmit, strlen(toTransmit), 0);

    if(charsWritten < 0)                                        // If it fails
    {
        error("CLIENT: ERROR writing to socket");               // Output error
    }

    memset(str, '\0', sizeof(str));                             // Clear buffer

    // Recieve the transmission from the server
    charsRead = recv(socketHandle, str, sizeof(str) - 1, 0);
    if(charsRead < 0)                                           // If the recieve fails
    {
        error("CLIENT: ERROR reading from socket");             // Output error
    }

    // Output the recieved chars
    printf("%s\n", str);
    fflush( stdout );

    close(socketHandle);                                        // Hang up
    return 0;
}