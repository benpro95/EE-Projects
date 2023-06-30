#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

// Read from and write to the serial port
char read_buffer[256];
int serial_port;
const char device[] = "/dev/ttyACM0";  // Update with your serial port device name

char *line =NULL;
size_t len = 0;
ssize_t lineSize = 0;

void readIn() {
  len = 0;
  lineSize = 0;
  line = realloc(line, 0);
  printf("Please enter a line:\n");
  lineSize = getline(&line, &len, stdin);
}


int serialWrite() {

    char linecmd[4] = "0";
    char delay[4] = "128";
    char sermsg[lineSize];
    char rawdata[128] = "<";
    
    int serline = 0;
    for(int _idx = 0; _idx < lineSize - 1; _idx++) { 
      // copy line array
      sermsg[serline] = line[serline];
      serline++;
    }
    sermsg[serline] = '\0';

    // build output string
    strcat(rawdata, linecmd);  
    strcat(rawdata, ","); 
    strcat(rawdata, delay);    
    strcat(rawdata, ",");   
    strcat(rawdata, sermsg);  
    strcat(rawdata, ">");  
    printf("%s\n", rawdata);

    // Write to the serial port
    write(serial_port, rawdata, sizeof(rawdata));  

    // Read from the serial port in a loop
    char target_char = '*';
    
    while (1) {
        // continue to read input
        readIn();
        // Check if the serial port is available
        if (access(device, F_OK) != 0) {
            printf("Serial port not available\n");
            return 1;
        }
        // Read from the serial port
        int num_bytes = read(serial_port, read_buffer, sizeof(read_buffer));
        if (num_bytes < 0) {
            printf("Error reading from serial port\n");
            return 1;
        }
        // Check if the target character is received
        int i;
        for (i = 0; i < num_bytes; i++) {
            if (read_buffer[i] == target_char) {
                printf("Received target character '%c'\n", target_char);
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

  while (1) {
    readIn();
    printf("You entered %s, which has %zu chars.\n", line, lineSize-1);
    status = serialWrite();
    printf("LOOP\n");  
    if (status > 0) {
      // Close the serial port
      close(serial_port);
      break;
    }
  }
  return status;

}