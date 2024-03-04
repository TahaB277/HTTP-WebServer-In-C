#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    char Buffer[BUFFER_SIZE]; // Used for reading from the connected socket 
    char toSend[] = "HTTP/1.0 200 OK\r\n"            // HTTP protocol, used for writing to the connected socket
                  "Server: webServer-c\r\n"
                  "Content-type: text/html\r\n\r\n"
                  "<html>Hello, World!</html>\r\n";
    // Creating the socket
    int mySocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (mySocket == -1){
        perror("Webserver error in socket");
        return 1;

    }
    printf("Socket created successfully\n");
    // Create client address
    struct sockaddr_in clientAddress;
    int clientAddresslen = sizeof(clientAddress);
    // Creating the host address
    struct sockaddr_in hostAddress;
    int hostAddresslen = sizeof(hostAddress);

    hostAddress.sin_family = AF_INET;
    hostAddress.sin_port = htons(PORT); // htons() converts the host byte order to network byte order (distinguish between little and big endian)
    hostAddress.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY is 0.0.0.0 which means 'any address' (its already in netork bytes but the man
    // Binding the socket to the address              //   page suggest to convert it)
    if (bind(mySocket, (struct sockaddr *)&hostAddress, hostAddresslen) != 0) // 0 if success, -1 otherise ---- for struct sockaddr see below ----
                                                                              // "The only purpose of this structure is to cast the structure
                                                                              //  pointe passed  in  addr in order to avoid compiler warnings"
    {
        perror("Webserver error in bind");
        return 1;
    }
    printf("socket successfully bound to address\n");
    // Listening for incoming connections
    if (listen(mySocket, SOMAXCONN) != 0) // Second argument is backlog (max nb of queue) SOMAXCONN is "Maximum queue length specifiable by 
    {                                     // listen" here its 4096 --- listen sets socket to passive 
        perror("Webserver error in listen");
        return 1;
    }
    printf("Server listening for connections\n");
    // Accepting inconing calls
    for(;;) // Infinite loop, same as while(true)
    {
        int fileDescriptor = accept(mySocket, (struct sockaddr *)&hostAddress, (socklen_t *)&hostAddresslen); // fileDescriptor refers to the new connected socket made by the fct accept
        if(fileDescriptor < 0) {
            perror("Webserver error in accept");
            return 1;
        }
        printf("Connection accepted\n");
        // Get client address
        int ClientSocket = getsockname(fileDescriptor, (struct sockaddr *)&clientAddress, (socklen_t *)&clientAddresslen);
        if (ClientSocket < 0) {
            perror("Webserver error in getsockname");
            continue;
        }
        // Reading from the socket
        int ReadVal = read(fileDescriptor, Buffer, BUFFER_SIZE); // where to put the read file (second arg) and how much to read (third arg)
        if (ReadVal < 0){
            perror("Webserver error in read");
            continue;
        }
        // Read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(Buffer, "%s %s %s", method, uri, version);
        printf("[%s:%u] %s %s %s\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), method, version, uri);
        // 1st fct converts to string, 2nd opposite of hton
        // Writing to the socket
        int WriteVal = write(fileDescriptor, toSend, strlen(toSend));
        if (WriteVal < 0){
            perror("Webserver error in write");
            continue;
        }

        close(fileDescriptor); // closes the fileDescriptor and makes it reusable
    }

    return 0;
}