#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdbool.h>

#define ROUND_DIVIDE(numer, denom) (((numer) + (denom) / 2) / (denom))

// Read from and write to the serial port
char read_buffer[256];
int serial_port;
const char device[] = "/dev/ttyACM0";  // Serial Port
size_t maxCmdLength = 21;
int writeloops = 0;
bool enableSend = 0;
char *line = NULL;
size_t lineSize = 0;



void readIn() {
    // configure standard in
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    char input;
        // When a character is detected
        if (read(STDIN_FILENO, &input, 1) == 1) {
            // allocate memory
            line = realloc(line, (lineSize + 1));
            if (input == '\n') { // Enter key is received
                // add null terminator
                line[lineSize] = '\0';
                // calcuate transmission rounds
                if (lineSize == 0) {
                  lineSize = 1;
                }
                writeloops = ROUND_DIVIDE(lineSize,maxCmdLength);
                writeloops++;
                lineSize = 0; // reset index
                // enable trasmit
                enableSend = 1; 
            } else {
                // Read terminal standard in
                printf("Read: %c\n", input);
                line[lineSize] = input; // write
                lineSize++; // increment index
            }
        }
  term.c_lflag |= ICANON | ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}


int serialWrite() {
    char linecmd[4] = "0";
    char delay[4] = "128";
    char rawdata[128] = "<";
    // build output string
    strcat(rawdata, linecmd);  
    strcat(rawdata, ","); 
    strcat(rawdata, delay);    
    strcat(rawdata, ",");   
    strcat(rawdata, line);  
    strcat(rawdata, ">");  
    // display final string
    printf("Serial Data: %s\n", rawdata);
    printf("Loops: %u\n", writeloops);
 
    // Check if the serial port is available
    if (access(device, F_OK) != 0) {
      printf("Serial port not available\n");
      return 1;
    }

    // Write to the serial port
    write(serial_port, rawdata, sizeof(rawdata));  
}

int serialRead() {
    // Check if the serial port is available
    if (access(device, F_OK) != 0) {
      printf("Serial port not available\n");
      return 1;
    }
    // Read from the serial port in a loop
    char target_char = '*';
    printf("Waiting for acknowledge...\n");
     while (1) {
        // Read from the serial port
        size_t num_bytes = read(serial_port, read_buffer, sizeof(read_buffer));
        if (num_bytes < 0) {
            printf("Error reading from serial port\n");
            return 1;
        }
        // Check if the target character is received
        size_t i;
        for (i = 0; i < num_bytes; i++) {
            if (read_buffer[i] == target_char) {
                printf("Received acknowledge '%c'\n", target_char);
                return 0;
            }
        }
        // Clear the read buffer
        memset(read_buffer, 0, sizeof(read_buffer));
        // Add a delay before the next read
        usleep(50000);  // 50ms delay
     }   
}


int main() {

    // Open the serial port
    serial_port = open(device, O_RDWR);
    if (serial_port < 0) {
        perror("Error opening serial port");
        return 1;
    }
    
    // Configure the serial port
    struct termios tty;
    if (tcgetattr(serial_port, &tty) != 0) {
        perror("Error configuring serial port");
        return 1;
    }
    
    // Set the baud rate (for example, 9600)
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);
    
    // Set other settings (8N1)
    tty.c_cflag &= ~PARENB;  // Disable parity bit
    tty.c_cflag &= ~CSTOPB;  // Set one stop bit
    tty.c_cflag &= ~CSIZE;   // Clear data size bits
    tty.c_cflag |= CS8;      // Set 8 data bits
    tty.c_cflag &= ~CRTSCTS; // Disable hardware flow control
    
    // Apply the settings
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        perror("Error applying serial port settings");
        return 1;
    }

  int status = 0;
  // wait for boot response
  status = serialRead();
  while (1) {
    // read standard line
    readIn();
    // transmit to serial
    if (enableSend == 1) {
       status = serialWrite();
       status = serialRead();
       enableSend = 0;
    }
    if (status == 1) {
      // Close the serial port
      close(serial_port);
      break;
    }
  }
  return 0;
}