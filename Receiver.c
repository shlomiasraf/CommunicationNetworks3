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
#include <netinet/tcp.h>

#define SERVER_IP "127.0.0.1"
int runNumber = 1;
int receiverSocket, senderSocket; // Variables for receiver and sender sockets
struct sockaddr_in serverAddr; // Server address structure
char buffer[1024] = {0}; // Buffer for receiving data
int fileSize = 0; // Variable to store file size
struct timeval start, end; // Variables to measure time
double totalTime = 0; // the total time from when the files were sent until they arrived
double totalBandwidth = 0.0; // Variable to store total bandwidth
double elapsedTimeArray[50]; // Assuming a maximum of 50 runs, adjust as needed
double bandwidthArray[50];   // Assuming a maximum of 50 runs, adjust as needed
int createTheSocket(int port, char* algo)
{
    // Create socket for the receiver
    receiverSocket = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
    if (receiverSocket == -1) { // Check for socket creation error
        perror("socket"); // Print error message
        exit(EXIT_FAILURE); // Exit program with failure status
    }
    //algorithm settings
    if (setsockopt(receiverSocket, IPPROTO_TCP, TCP_CONGESTION,algo,strlen(algo)))
    {
        perror("failed to set congestion control algorithm");
        return -1;
    }
    // Setup server address
    serverAddr.sin_family = AF_INET; // Set address family to IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Set IP address to localhost
    serverAddr.sin_port = htons(port); // Set port number

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
        printf("----------------------------------\n");
        printf("           * Statistics *         \n");
        for(int i = 0; i < runNumber; i++)
        {
            // Print time and bandwidth
            printf("runNumber: %d \n", i+1);
            printf("Received file in %.2f milliseconds.\n", elapsedTimeArray[i]); // Print elapsed time
            printf("Bandwidth: %.2f bytes/millisecond\n", bandwidthArray[i]); // Print bandwidth
        }
        printf("----------------------------------\n");
        printf("Average time: %.2f milliseconds\n", totalTime/runNumber);
        printf("Average bandwidth: %.2f bytes/millisecond\n", totalBandwidth/runNumber);
        printf("----------------------------------\n");
        printf("Receiver end.\n");      
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
    memset(buffer, 0, sizeof(buffer));
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
    clock_gettime(CLOCK_MONOTONIC, &end); // Get end time
    // Calculate elapsed time// Calculate elapsed time in milliseconds
    double elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    double currentBandwidth = (received / elapsedTime);
    // Update total bandwidth
    totalBandwidth += currentBandwidth; // Calculate bandwidth in bytes/ms
    totalTime += elapsedTime;
    elapsedTimeArray[runNumber - 1] = elapsedTime;
    bandwidthArray[runNumber - 1] = currentBandwidth;
    // Wait for sender response
    printf("Waiting for sender response...\n"); // Wait message
    receiveExitSignal(senderSocket);
}
int main(int argc, char *argv[])
{
    int port = atoi (argv[2]);
    char* algo = argv[4];
    createTheSocket(port,algo);
    while(1)
    {
        receiveTheFile(runNumber);
        runNumber++;
    }
}
