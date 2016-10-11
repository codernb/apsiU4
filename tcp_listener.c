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

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

//XXX: 2. missing include
#include <arpa/inet.h>

void on_error(int errorCode, char* message) {
  if (errorCode == -1) {
    perror(message);
    exit(-1);//XXX: 1. Unreleased resources on error
  }
}

int main () {  
  int serverSocket, instanceSocket;
  struct sockaddr_in  serverAddress, instanceAddress;
  int instanceAddressSize;
  char readBuffer[100], writeBuffer[100];
  int errorCode;
  int outputFileHandler, inputFileHandler;
  ssize_t inputLength, writeLength, readLength;
  //XXX: 4.1 out_len, never used

  char* outputFileName = "input_data";
  char* outputTempFileName = "input_data.tmp";
  char* inputFile = "result_data";
  
  //XXX: 4. never used char* in_file_tmp = "result_data.tmp";
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(8500);    
  serverAddress.sin_addr.s_addr = inet_addr("localhost");                                   
   
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  on_error(serverSocket, "\nSocket creation:\n"); 

  errorCode = bind(serverSocket, (struct sockaddr*) &serverAddress, 
	     sizeof(serverAddress));
  on_error(errorCode, "\n socket binding:\n");
  
  errorCode = listen(serverSocket, 1);
  on_error(errorCode, "\nQueue setting:\n");

  while (1) {//XXX: 3. never shuts down gracefully (leaving sockets open?)
    // accept requests, 
    instanceAddressSize = sizeof(instanceAddress);
    instanceSocket = accept(serverSocket, (struct sockaddr*) &instanceAddress,
                       &instanceAddressSize);
    on_error(instanceSocket, "\n socket instance creation:\n");

    inputLength = read(instanceSocket, &readBuffer, 100);
    on_error(inputLength, "error accepting connection");
    //XXX: jup, there is 3.
    if (inputLength == 0) {
      close(instanceSocket);
      continue; 
    }
    
    outputFileHandler = open(outputTempFileName, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
    on_error(outputFileHandler, "error creating output file");
    
    writeLength = write(outputFileHandler, readBuffer, inputLength);
    on_error(writeLength, "error creating output file");
    
    if (writeLength != inputLength) {
      fprintf(stderr, "short write to disk");
      exit(-1);
    }
    
    errorCode = close(outputFileHandler);
    on_error(errorCode, "error creating output file");

    errorCode = rename(outputTempFileName, outputFileName);  
    on_error(errorCode, "error renaming file");    
    
    while (1) {
      inputFileHandler = open(inputFile, O_RDONLY);
      if (inputFileHandler == -1 && errno == ENOENT) {
        // file does not exist
        sleep(1);
        continue;
      }
      on_error(inputFileHandler, "error opening input file");
      break;
    }

    readLength = read(inputFileHandler, writeBuffer, 100);
    on_error(readLength, "error reading file");
    
    errorCode = close(inputFileHandler);
    on_error(errorCode, "error closing file after read");

    errorCode = unlink(inputFile);
    on_error(errorCode, "error unlinking input file");
    
    write(instanceSocket, &writeBuffer, readLength);
    close(instanceSocket);               // clean up
  }  
  close(serverSocket);                // clean up
}
  
