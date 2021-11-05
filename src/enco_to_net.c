#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define MULTICASTADDR "224.0.0.69"
#define MULTICASTPORT 10001
#define ENCODERSBAUDRATE 56700

int open_port(char *device)
{
    int fd;
    struct termios io;

    fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd > 0)
    {
        memset (&io, 0, sizeof(io));

        // Baudrate
        cfsetispeed(&io, ENCODERSBAUDRATE);
        cfsetospeed(&io, ENCODERSBAUDRATE);

        // Enable receiver and set local mode
        io.c_cflag |= (CLOCAL | CREAD);

        // No parity (8N1)
        io.c_cflag &= ~PARENB;
        io.c_cflag &= ~CSTOPB;
        io.c_cflag &= ~CSIZE;
        io.c_cflag |= CS8;

        io.c_cflag &= ~CRTSCTS;

        // Set the new options for the port
        tcsetattr(fd, TCSANOW, &io);

        // Flush Buffer
        usleep(500);
        tcflush(fd, TCIOFLUSH);


    }
    return 0;
}

int open_socket()
{
    int sock;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    return 0;
}

int main (int argc, char **argv)
{
    int fd, op;
    char *filename;  

    while ((op = getopt(argc, argv, "d:")) != EOF)
    {
        switch (op)
        {
            case 'd':
                filename = argv[optind-1];
                fd = open_port(filename);
                if (fd < 0){
                    exit("Error opening the device");
                }
                break;
            
            default:
                exit(-1);
        }

    }
    return 0;
}