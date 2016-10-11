// Binds to a port, accepts the first connection and 
// sends one input line onwards via out_file.
// Waits then for a response via in_file and,
// when that arrives echos that to the user and closes 
// the connection.

// Client: Use, e.g. telnet 127.0.0.1 8500
//              or   telnet localhost 8500
//         on the same machine.
//
// Note:   To exit a telnet-session give <crtl>-] 
//         and then the command "quit"
//         or simply press Ctrs + AltGr + 9

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>

const char* outputFileName = "input_data";
const char* outputTempFileName = "input_data.tmp";
const char* inputFileName = "result_data";

struct sockaddr_in serverSocketAddress, instanceAddress;
int serverSocket, instanceSocket;
int instanceAddressSize = sizeof(instanceAddress);

char readBuffer[100], writeBuffer[100];
int outputFileHandler, inputFileHandler;
ssize_t inputLength, writeLength, readLength;

void cleanUp() {
    close(instanceSocket);
    close(serverSocket);
    close(outputFileHandler);
    close(inputFileHandler);
    unlink(outputFileName);
    unlink(inputFileName);
}

void check(int errorCode, char* message) {
    if (errorCode != -1)
        return;
    perror(message);
    cleanUp();
    exit(-1);
}

void setUpServerSocketAddress() {
    serverSocketAddress.sin_family = AF_INET;
    serverSocketAddress.sin_port = htons(8500);    
    serverSocketAddress.sin_addr.s_addr = inet_addr("127.0.0.1");                                   
}

void setUpServerSocket() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    check(serverSocket, "\nSocket creation:\n"); 
}

void bindServerSocket() {
    check(bind(serverSocket, (struct sockaddr*) &serverSocketAddress, sizeof(serverSocketAddress)), "\n socket binding:\n");
    check(listen(serverSocket, 1), "\nQueue setting:\n");
}

void acceptRequest() {
    instanceSocket = accept(serverSocket, (struct sockaddr*) &instanceAddress, &instanceAddressSize);
    check(instanceSocket, "\n socket instance creation:\n");
}

void readInput() {
    inputLength = read(instanceSocket, &readBuffer, 100);
    check(inputLength, "error accepting connection");
}

int validateInput() {
    return inputLength > 2;
}

void openOutputFile() {
    outputFileHandler = open(outputTempFileName, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
    check(outputFileHandler, "error creating output file");
}

void writeToOutputFile() {
    writeLength = write(outputFileHandler, readBuffer, inputLength);
    check(writeLength, "error creating output file");
    if (writeLength != inputLength)
        check(-1, "short write to disk");
}

void closeOutputFile() {
    check(close(outputFileHandler), "error creating output file");
}

void renameOutputFile() {
    check(rename(outputTempFileName, outputFileName), "error renaming file"); 
}

void unlinkOutputFile() {
    check(unlink(outputFileName), "error unlinking output file");
}

void openInputFile() {
    while (1) {
        inputFileHandler = open(inputFileName, O_RDONLY);
        if (inputFileHandler == -1 && errno == ENOENT) {
            sleep(1);
            continue;
        }
        check(inputFileHandler, "error opening input file");
        break;
    }
}

void readInputFile() {
    readLength = read(inputFileHandler, writeBuffer, 100);
    check(readLength, "error reading file");
}

void closeInputFile() {
    check(close(inputFileHandler), "error closing file after read");
}

void unlinkInputFile() {
    check(unlink(inputFileName), "error unlinking input file");
}

void writeAnswer() {
    write(instanceSocket, &writeBuffer, readLength);
}

void endRequest() {
    close(instanceSocket);
}

void handleRequests() {
    while (1) {
        acceptRequest();
        readInput();
        if (!validateInput()) {
            close(instanceSocket);
            continue; 
        }
        openOutputFile();
        writeToOutputFile();
        closeOutputFile();
        renameOutputFile();
        openInputFile();
        readInputFile();
        closeInputFile();
        unlinkInputFile();
        writeAnswer();
        endRequest();
    }
}

int main () {
    setUpServerSocketAddress();
    setUpServerSocket();
    bindServerSocket();
    handleRequests();
}

