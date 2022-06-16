#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

// TODO Code encryption function and add key stuff

char * decrypt(char * msg, char * key)
{
    char decMsg[70000];                                     // String to store decrypted msg
    strcpy(decMsg, msg);                                    // Copy message to decMsg

    for(int i = 0; i < strlen(decMsg); i++)                 // Iterate the message
    {
        if(decMsg[i] == 32)                                 // If it has a space
            decMsg[i] = 91;                                 // Turn it into a bracket
        if(key[i] == 32)                                    // If key has a space
            key[i] = 91;                                    // Turn it into a bracket
        int decChar = (decMsg[i]) - key[i] + 65;            // Logic for encrypting
        if(decChar < 65)                                    // Modulate to correct range
            decChar = decChar + 27;                         // Here
        if(decChar == 91)                                   // If it turns into a bracket
            decChar = 32;                                   // Make a space
        decMsg[i] = (char)decChar;                          // Store
    }

    return decMsg;                                          // Return
}

// Error function used for reporting issues
void error(const char * msg)
{
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket

void setupAddressStruct(struct sockaddr_in * address, int portNumber)
{
    memset((char *) address, '\0', sizeof(*address));           // Clear address struct
    address->sin_family = AF_INET;                              // Make sure its network capable
    address->sin_port = htons(portNumber);                      // Store the port number
    address->sin_addr.s_addr = INADDR_ANY;                      // Allow clients of any address to connect
}

int main(int argc, char * argv[])
{
    int connectionSocket;
    int charsRead;
    int charsWritten;
    char buffer[150000];
    char buffer2[150000];
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    if(argc < 2)                                                // If too few arguments
    {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);           // Output errors
        fflush( stdout );
        exit(1);                                                // Exit with code 1
    }

    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);         // Instantiate the socket to listen for connections
    if(listenSocket < 0)                                        // If it didn't work
    {
        error("ERROR opening socket");                          // Output errors
    }

    setupAddressStruct(&serverAddress, atoi(argv[1]));           // Set up the address struct for the server socket

    // Bind the socket to the port
    if(bind(listenSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        error("ERROR on binding");                              // Output error if it failed
    }

    listen(listenSocket, 5);                                    // Begin listening for connections, and allow up to 5

    while(1)
    {
        // Accept the connection and grab client address info
        connectionSocket = accept(listenSocket, (struct sockaddr *) &clientAddress, &sizeOfClientInfo);
        if(connectionSocket < 0)                                // If it failed to accept
        {
            error("ERROR on accept");                           // Output error
        }

        pid_t childPID = -1;                                    // Child PID
        int childExitStatus = -1;                               // Child exit status
        childPID = fork();                                      // Go forth child, and do my bidding
        
        switch(childPID)
        {
            case -1:                                            // If error
                error("Invalid child!\n");                      // Call....errror
                break;
            case 0:
                // Debug output 
                printf("SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));
                fflush( stdout );
                memset(buffer, '\0', 150000);                              // Clear buffer
                memset(buffer2, '\0', 150000);                             // Dewit again

                charsRead = recv(connectionSocket, buffer2, sizeof(buffer2), 0);     // Recieve communication into the buffer of max size 255
                strcat(buffer, buffer2);                                    // Add to buffer
                if(charsRead > 80000)                                       // If its a bigun
                {
                    memset(buffer2, '\0', 150000);                          // Clear the buffer2
                    // Come again
                    charsRead = recv(connectionSocket, buffer2, sizeof(buffer2), 0);
                    strcat(buffer, buffer2);                                // Slap on the end
                }
                if(charsRead < 0)                                           // If the recieve failed
                {
                    error("ERROR reading from socket");                     // Output error
                }

                char msg[70000];                                            // String for msg
                char key[80000];                                            // String for key
                char ID[2];                                                 // String for ID
                char * token = strtok(buffer, "!");                         // Grab msg
                strcpy(msg, token);                                         // Store
                token = strtok(NULL, "!");                                  // Grab key
                strcpy(key, token);                                         // Store
                token = strtok(NULL, "!");                                  // Grab ID
                strcpy(ID, token);                                          // Store
                if(strncmp(ID, "D", 1) != 0)                                // If its the wrong client
                {
                    close(connectionSocket);                                // Close connection
                    // Error output
                    error("SERVER: INVALID client connecting, terminated connection\n");
                }

                if(strlen(msg) > strlen(key))                               // If key isnt long enough
                {
                    error("DEC_SERVER: ERROR msg is longer than key\n");    // Error output
                }

                char * encMsg = decrypt(msg, key);                          // Decrypt msg

                // Send it back
                charsWritten = send(connectionSocket, encMsg, strlen(encMsg), 0);
                fflush( stdout );
                if(charsWritten < 0)                                        // If the message failed to send
                {
                    error("ERROR writing to socket");                       // Output error
                }
                exit(0);
                break;
            default: ;
                pid_t temp = waitpid(childPID, &childExitStatus, WNOHANG);  // Wait for child to finish bidding
                close(connectionSocket);                                    // Close the connection
                break;
        }

    }

    close(listenSocket);                                                    // Close listen connection
    return 0;
}