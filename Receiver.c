#include <stdio.h> // Standard input-output header
#include <stdlib.h> // Standard library header
#include <sys/socket.h> // Socket programming header
#include <netinet/in.h> // Internet address family header
#include <string.h> // String manipulation header
#include <unistd.h> // POSIX operating system API header
#include <sys/time.h> // Time functions header
#include <arpa/inet.h>
#include <bits/time.h>
#include <time.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
int runNumber = 1;
int receiverSocket, senderSocket; // Variables for receiver and sender sockets
struct sockaddr_in serverAddr; // Server address structure
char buffer[1024] = {0}; // Buffer for receiving data
int fileSize = 0; // Variable to store file size
struct timeval start, end; // Variables to measure time
double elapsedTime; // Variable to store elapsed time
double totalBandwidth = 0.0; // Variable to store total bandwidth
int fileCount = 0; // Variable to count the number of files received
double totalTime = 0; // the total time from when the files were sent until they arrived
int createTheSocket()
{
    // Create socket for the receiver
    receiverSocket = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
    if (receiverSocket == -1) { // Check for socket creation error
        perror("socket"); // Print error message
        exit(EXIT_FAILURE); // Exit program with failure status
    }

    // Setup server address
    serverAddr.sin_family = AF_INET; // Set address family to IPv4
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP); // Set IP address to localhost
    serverAddr.sin_port = htons(PORT); // Set port number

    // Bind socket to port
    if (bind(receiverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1) { // Bind socket to port
        perror("bind"); // Print error message
        exit(EXIT_FAILURE); // Exit program with failure status
    }
    printf("Waiting for TCP connection...\n");
    // Listen for incoming connections
    if (listen(receiverSocket, 5) == -1) { // Listen for incoming connections
        perror("listen"); // Print error message
        exit(EXIT_FAILURE); // Exit program with failure status
    }
    // Infinite loop to handle multiple connections
        // Accept connection from sender
        socklen_t addrLen = sizeof(serverAddr); // Length of server address structure
        senderSocket = accept(receiverSocket, (struct sockaddr *) &serverAddr, &addrLen); // Accept connection
        if (senderSocket == -1)
        { // Check for accept error
            perror("accept"); // Print error message
            exit(EXIT_FAILURE); // Exit program with failure status
        }
        printf("Connection accepted from sender.\n"); // Connection accepted message
}
void receiveExitSignal(int senderSocket)
{
    int exitSignal;
    recv(senderSocket, &exitSignal, sizeof(int), 0);
    if (exitSignal == -1)
    {
        printf("Received exit signal from server.\nClosing connection.\n");
        printf("Average time: %.2f milliseconds\n", totalTime/runNumber);
        printf("Average bandwidth: %.2f bytes/millisecond\n", totalBandwidth/runNumber);
        // Handle the exit signal as needed
        close(senderSocket);
        exit(EXIT_SUCCESS);
    }
}
void receiveTheFile(int runNumber)
{
    int received = recv(senderSocket, &fileSize, sizeof(int), 0); // Receive file size
    // Receive file size
    if (received == -1)
    { // Check for receive error
        perror("rec"); // Print error message
        exit(EXIT_FAILURE); // Exit program with failure status
    }
    // Receive file content
    clock_gettime(CLOCK_MONOTONIC, &start); // Get start time
    received = recv(senderSocket, buffer, fileSize, 0); // Receive file content
    if (received == -1)
    { // Check for receive error
        perror("recv"); // Print error message
        exit(EXIT_FAILURE); // Exit program with failure status
    }
    else if (received == 0)
    { // Check if sender closed connection
        return; // Exit loop
    }
    clock_gettime(CLOCK_MONOTONIC, &end); // Get start time
    // Calculate elapsed time
    elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0; // Convert seconds to milliseconds
    elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0; // Convert microseconds to milliseconds

    // Update total bandwidth
    totalBandwidth += (fileSize / elapsedTime); // Calculate bandwidth in bytes/ms
    // Print time and bandwidth
    printf("runNumber: %d \n", runNumber);
    printf("Received file in %.2f milliseconds.\n", elapsedTime); // Print elapsed time
    totalTime += elapsedTime;
    printf("Bandwidth: %.2f bytes/millisecond\n", fileSize / elapsedTime); // Print bandwidth
    // Increase file count
    fileCount++; // Increment file count
    // Wait for sender response
    printf("Waiting for sender response...\n"); // Wait message
    receiveExitSignal(senderSocket);
}
int main()
{
    createTheSocket();
    while(1)
    {
        receiveTheFile(runNumber);
        runNumber++;
    }
}