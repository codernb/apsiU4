// Looks for new input file, reads it deletes it
// Performs service on it: To upper case
// Note: The only valid input is 3 lowercase letters!
// Writes result to output file


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char* root = ".";
const char* inputFileName = "input_data";
const char* outputFileName = "result_data";
const char* outputTempFileName = "result_data.tmp";

char readWriteBuffer[101];
int i;
int inputFileHandler, outputFileHandler;
ssize_t readLength;

void cleanUp() {
}

void check(int errorCode, char* message) {
    if (errorCode != -1)
        return;
    perror(message);
    cleanUp();
    exit(-1);
}

void on_error(int err, char * msg) {
  if (err == -1) {
    perror(msg);
    exit(-1);
  }
}

void dropPrivileges() {
    struct stat directoryStatus;
    chroot(root);
    chdir(root);
    stat(root, &directoryStatus);
    check(setgid(directoryStatus.st_gid), "error setgid");
    check(setuid(directoryStatus.st_uid), "error setuid");
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
    readLength = read(inputFileHandler, readWriteBuffer, 100);
    check(readLength, "error reading file");
    readLength--;
}

void closeInputFile() {
    check(close(inputFileHandler), "error closing file after read");
}

void unlinkInputFile() {
    check(unlink(inputFileName), "error unlinking input file");
}

void manipulateInput() {
    for (i = 0; i < readLength; i++)
        readWriteBuffer[i] = readWriteBuffer[i] - 32;
}

void openOutputFile() {
    outputFileHandler = open(outputTempFileName, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
    check(outputFileHandler, "error creating output file");
}

void writeToOutputFile() {
    int writeLength = write(outputFileHandler, readWriteBuffer, readLength);
    check(writeLength, "error creating output file");
    if (writeLength != readLength)
        check(-1, "short write to disk");
}

void closeOutputFile() {
    check(close(outputFileHandler), "error creating output file");
}

void renameOutputFile() {
    check(rename(outputTempFileName, outputFileName), "error renaming file"); 
}

int main () {
    dropPrivileges();
    while (1) {
        openInputFile();
        readInputFile();
        closeInputFile();
        unlinkInputFile();
        manipulateInput();
        openOutputFile();
        writeToOutputFile();
        closeOutputFile();
        renameOutputFile();
    }
}  
  
  

