#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>


char* fileName = "file.txt";
char* CC_cubic = "cubic";
char* CC_reno = "reno";
int senderSocket;
int fileSize = 0;
char *fileContent = NULL;

void readTheFile()
{
    printf("Server startup\n");

    printf("Reading file...\n");
    FILE *fpointer = fopen(fileName, "r"); // Open the file for reading

    if (fpointer == NULL)
    { // Check if the file was successfully opened
        perror("fopen"); // Print an error message if fopen fails
        exit(EXIT_FAILURE); // Exit the program with failure status
    }

    fseek(fpointer, 0L, SEEK_END); // Move the file pointer to the end of the file
    fileSize = ftell(fpointer);    // Get the size of the file
    fseek(fpointer, 0L, SEEK_SET); // Reset the file pointer to the beginning of the file

    fileContent = (char *)malloc(fileSize * sizeof(char)); // Allocate memory for the file content
    if (fileContent == NULL)
    {                      // Check if memory allocation fails
        perror("malloc");  // Print an error message if malloc fails
        fclose(fpointer);  // Close the file
        exit(EXIT_FAILURE); // Exit the program with failure status
    }
    memset(fileContent, 0, fileSize);
    fread(fileContent, sizeof(char), fileSize, fpointer); // Read the file content into memory
    fclose(fpointer); // Close the file
}

void createTheSocket(int port,char* algo,char* ip_address)
{
    // create socket
    printf("Setting up the socket...\n");
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    // Create socket for the sender
    senderSocket = socket(AF_INET, SOCK_STREAM, 0);
    // Set congestion control algorithm to Reno
    //algorithm settings
    if (setsockopt(senderSocket, IPPROTO_TCP, TCP_CONGESTION,algo,strlen(algo)))
    {
        perror("failed to set congestion control algorithm");
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip_address);
    serverAddr.sin_port = htons(port);

    // Connect to the receiver
    int bond = connect(senderSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (bond == -1)
    {
        perror("connect");
        // Handle error
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Connected to receiver.\n");
    }
}

void sendTheFile()
{
    int sent = send(senderSocket, fileContent, fileSize, 0);
    if (sent == -1)
    {
        perror("send");
        // Handle error
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("File content sent successfully.\n");
    }
}

void closeConnection()
{
    // exit the program
    printf("Closing connection...\n");
    int exitSignal = -1;
    send(senderSocket, &exitSignal, sizeof(int), 0);
    // close socket
    close(senderSocket);
}

int main(int argc, char *argv[])
{
    int port = atoi (argv[4]);
    char* algo = argv[6];
    char* ip_address = argv[2];
    readTheFile();
    createTheSocket(port,algo,ip_address);
    sendTheFile();
    printf("do you want to send the file again?\n");
    char word[4];
    scanf("%3s", word);
    while (strcmp(word, "no") != 0 && strcmp(word, "yes") != 0)
    {
        printf("please enter yes/no\n");
        scanf("%3s", word);
    }
    while (strcmp(word, "no") != 0)
    {
        sendTheFile();
        printf("do you want to send the file again?\n");
        scanf("%3s", word);
        while (strcmp(word, "no") != 0 && strcmp(word, "yes") != 0)
        {
            printf("please enter yes/no\n");
            scanf("%3s", word);
        }
    }
    closeConnection();
    return 0; // Return success
}
