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

#include <encoders.h>


#define MULTICASTADDR    "224.0.0.69"
#define MULTICASTPORT    10001
#define ENCODERSBAUDRATE 56700
#define DEVICE           "/dev/encoder"
#define BUFFLEN          128
#define ENCOPACKETLEN    38

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

int open_socket(char *address, int port, struct sockaddr_in *addr)
{
    int sock;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(address);
    addr->sin_port = htons(port);

    return sock;
}

/// Read con timeout por software
ssize_t soft_read_time_out(struct timeval tvl, int fd, void *buf,
                           size_t count)
{
    fd_set rfds;
    int fd_max = fd + 1;
    int retval, l = 0;

    while (l < count)
    {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        retval = select(fd_max, &rfds, NULL, NULL, &tvl);
        if (retval) {
            if(FD_ISSET(fd, &rfds))
            {
                retval = read(fd, buf+l, count - l);
                l += retval;
            }
        }
        else
            return -1;
    }
    return l;
}

void network_relay(int fd, int socket, struct sockaddr *addr)
{
    struct timeval t;
    uint8_t buff[BUFFLEN];
    int res;

    t.tv_sec  = 0;
    t.tv_usec = 200000;

    while (1)
    {
        res = soft_read_time_out(t, fd, &buff, ENCOPACKETLEN);
        printf("Read: %d bytes", res);
        int nbytes = sendto(socket, buff, sizeof(buff), 0, addr, 
                     sizeof(*addr));    }
}


int main (int argc, char **argv)
{
    int fd, op, port, sock;
    char *filename, *address;
    struct sockaddr_in addr;
    const char *message = "Hola tarola";

    while ((op = getopt(argc, argv, "d:n:p:")) != EOF)
    {
        switch (op)
        {
            case 'd':
                filename = argv[optind-1];
                fd = open_port(filename);
                if (fd < 0){
                    exit(-1);
                }
                break;
            case 'n':
                address = argv[optind-1];    
            break;
            case 'p':
                port = atoi(argv[optind-1]);
            break;
            
            default:
                exit(-1);
        }
    }
    address = MULTICASTADDR;
    port = MULTICASTPORT;
    sock = open_socket(address, port, &addr);

    network_relay(fd, sock, &addr);

/*
    while (1)
    {

        int nbytes = sendto(sock, message, strlen(message), 0, (struct sockaddr*) &addr, sizeof(addr));
        printf("Written %d bytes\n", nbytes);
        sleep(1);
    }
*/
    return 0;
}