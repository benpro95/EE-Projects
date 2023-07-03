/// Z-Terminal Serial Communication v1.0
/// for GNU/Linux kernel versions 5.0+  
/// by Ben Provenzano III - 07/01/2023

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

// divide to whole number macro
#define ROUND_DIVIDE(numer, denom) (((numer) + (denom) / 2) / (denom))
// max buffer length
#define buffLen 128

// serial device
int serial_port;
const char device[] = "/dev/ttyACM1"; 
// max serial data chunk bytes
const size_t maxCmdLength = 64;
// signature matching
const char sigChars[] = {"$?-|"}; // control mode
size_t sigMatches = 0;
size_t sigLen = 0;
// control data
char controlDat[] = "0000\0";
size_t controlLen = 0;
size_t controlCount = 0;
bool controlMode = 0;
// line output data
char *line = NULL;
int writeLoops = 0;
size_t lineSize = 0;
size_t serCharBufSize = buffLen;
char serCharBuf[buffLen];
char rawData[buffLen];
char chunkBuf[buffLen];
// delay data
size_t delayint = 400; // default delay (ms)
char delaystr[8];
// flags
bool enableSend = 0;
bool resetLine = 0;


void resetSerial() {
  // send clear serial command
  rawData[0]='\0';
  strcat(rawData, "<3,0,0>"); 
  printf("Clear Data: %s\n", rawData);
  write(serial_port, rawData, sizeof(rawData));
  memset(rawData, 0, sizeof(rawData));
  resetLine = 0;
  // clear line data
  enableSend = 0;
  writeLoops = 0;
  line = realloc(line,1);
  line[0] = '\0';
  lineSize = 0;
}


void readIn(int _block) {
  // configure standard in
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  if (_block == 0) {
    // make stdin non-blocking
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  } else {
    // make stdin blocking
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
  }
  char input;
  // when a character is detected
  ssize_t bytesRead = read(STDIN_FILENO, &input, 1);
  if (bytesRead == 1) {
    if (enableSend == 0) {
      // allocate memory
      line = realloc(line, (lineSize + 1));
    }
    //// enter key is pressed ////
    if (input == '\n') { 
      if (controlMode == 0) {
        if (enableSend == 0) {
          // calcuate transmission rounds
          if (lineSize <= 0) {
            lineSize = 1;
          } 
          writeLoops = (ROUND_DIVIDE(lineSize,maxCmdLength) + 1);
          // enable transmit
          enableSend = 1;
          // terminate line
          line[lineSize] = '\0';
        }  
      } else { // control mode 
        printf("Control Data: %s\n", controlDat);
        resetSerial();
      } 
      // reset control mode
      controlMode = 0;
      controlCount = 0;
      sigMatches = 0;
      lineSize = 0;      
    } else {
    //// any other character is read ////
      if (enableSend == 0) {
        // write to line data array
        line[lineSize] = input; 
      }
      // detect control mode signature
      if (lineSize < sigLen) {
        // count matched characters
        if (input == sigChars[sigMatches]) {
          sigMatches = sigMatches + 1; 
          if (sigMatches >= sigLen){
            // all characters matched
            controlMode = 1;
          } 
        }  
      } // store control data after signature
      if (controlMode == 1) { 
        if (lineSize >= sigLen) { // skip over last signature char
          if (lineSize < (sigLen + controlLen)) { // do not allow overflow 
            // write each character to array
            controlDat[controlCount] = input;
            controlDat[controlCount + 1] = '\0';
          } // increment index
          controlCount++;
        }
      }
      // increment index
      lineSize++; 
      //printf("Read: %c\n", input);
    }
  }
  // reset standard in
  term.c_lflag |= ICANON | ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}


int setSerialNonBlock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
      return -1;
  }
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


int serialRead() {
 // Read from the serial port in a loop
 char target_char = '*';
 printf("Waiting for acknowledge...\n");
 while(1) {
    // set serial port to non-blocking mode
    if (setSerialNonBlock(serial_port) == -1) {
      perror("Failed to set serial port to non-blocking mode");
      return 1;
    }
    // read serial port
    ssize_t num_bytes = read(serial_port, serCharBuf, serCharBufSize);
    // check if the serial port is available
    if (access(device, F_OK) != 0) {
      printf("Serial port not available\n");
      return 1;
    }
    if (num_bytes > 0) {
      // Check if the target character is received
      size_t i;
      for (i = 0; i < num_bytes; i++) {
        // check for input
        if (serCharBuf[i] == target_char) {
            printf("Received acknowledge '%c'\n", target_char);
            return 0;
        }
      }
    } else {
      // no data available, continue reading input
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // continue to read input (non-blocking)
        readIn(0);
        if (resetLine == 1) {
          resetSerial();
          return 0;
        } 
      } else {
        // error occurred
        printf("Failed to read serial port\n");
        return 1;
      }
    }
  // us delay
  usleep(5000); 
  }
}


int serialWrite() {
  int status = 0;
  // check if the serial port is available
  if (access(device, F_OK) != 0) {
    printf("Serial port not available\n");
    return 1;
  }
  int i;
  size_t startpos = 0;
  // write max-char segments
  for (i = 1; i <= writeLoops; ++i) {
    // read / convert delay data
    sprintf(delaystr, "%zu", delayint);
    // build output string
    rawData[0]='<';
    strcat(rawData, "0,"); 
    strcat(rawData, delaystr);    
    strcat(rawData, ",");
    if (writeLoops > 1) {
      strncpy(chunkBuf,line+(startpos),maxCmdLength);
      startpos = startpos + maxCmdLength;
      strcat(rawData, chunkBuf); 
      strcat(rawData, ">"); 
    } else {
      strcat(rawData, line);  
      strcat(rawData, ">"); 
    }
    printf("Serial Data: %s\n", rawData);
    // write to the serial port
    write(serial_port, rawData, sizeof(rawData)); 
    // deallocate buffers
    memset(rawData, 0, sizeof(rawData));
    memset(chunkBuf, 0, sizeof(chunkBuf));
    // wait for response
    status = serialRead();
    // exit loop if reset
    if (enableSend == 0) {
      break;
    }      
  }
  return status;
}


int main() {
  // open the serial port
  serial_port = open(device, O_RDWR);
  if (serial_port < 0) {
    perror("Error opening serial port");
    return 1;
  }
  // configure the serial port
  struct termios tty;
  if (tcgetattr(serial_port, &tty) != 0) {
    perror("Error configuring serial port");
    return 1;
  }
  // set the baud rate (for example, 9600)
  cfsetospeed(&tty, B9600);
  cfsetispeed(&tty, B9600);
  // set other settings (8N1)
  tty.c_cflag &= ~PARENB;  // Disable parity bit
  tty.c_cflag &= ~CSTOPB;  // Set one stop bit
  tty.c_cflag &= ~CSIZE;   // Clear data size bits
  tty.c_cflag |= CS8;      // Set 8 data bits
  tty.c_cflag &= ~CRTSCTS; // Disable hardware flow control
  // apply the settings
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    perror("Error applying serial port settings");
    return 1;
  }
  controlLen = (sizeof(controlDat) - 2);
  sigLen = (sizeof(sigChars) - 1);
  // program status 
  int status = 0;
  printf("Z-Terminal Xmit v1.0\n");
  // wait for boot response
  status = serialRead();
  // main loop
  while(1) {
    // read standard line (blocking)
    readIn(1);
    // transmit to serial
    if (enableSend == 1) {
       status = serialWrite();
       enableSend = 0;
    }
    // clear display and reset
    if (resetLine == 1) {
      resetSerial();
    }
    // error occured
    if (status == 1) {
      // close the serial port
      close(serial_port);
      break;
    }
  }
  return status;
}