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

/*
 * swap: swaps a memory chunk
 *       callit: swap(&variable, sizeof(variable));
 */
void swap (void *data, size_t n)
{
	uint8_t temp;
	size_t i;
	n--;
	
	for (i = 0; i <= n/2; i++)
	{
		temp                    = *(uint8_t *) (data+i);
		*(uint8_t *) (data+i)   = *(uint8_t *) (data+n-i);
		*(uint8_t *) (data+n-i) = temp;
	}
}

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
    return fd;
}

int open_socket(char *address, int port, struct sockaddr_in *addr)
{
    int sock;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Error opening socket.");
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

    //tcflush(fd, TCIOFLUSH);

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

int wait_char(int fd, const u_char pattern)
{
    int ret = 0;
    u_char c;

    struct timeval tv = {
            .tv_sec = 1,
            .tv_usec = 0
    };

    do {
        ret = soft_read_time_out (tv, fd, &c, 1);
    } while ((ret == 1) && c != pattern);
    if (ret == 1) ret = 0;

    return ret;
}

void network_relay(int fd, int socket, struct sockaddr *addr, uint16_t packetid, 
                   float decoffset, float haoffset)
{
    struct timeval t;
    uint8_t buff[BUFFLEN];
    char datastring[ENCOPACKETLEN];
    int res;
    float dec, ha;
    int countsdec, countsha;
    char tempstring[6];

    struct SAO_data_transport sao_packet, sao_packet_net;

    sao_packet.syncword           = SYNCWORD;
    sao_packet.hdr.version        = VERSION;
    sao_packet.hdr.packetid       = packetid;
    sao_packet.hdr.message_type   = REPORTPACKET;
    sao_packet.hdr.packet_counter = 0;
    sao_packet.hdr.pdl            = sizeof(struct SAO_data_transport_payload);
    sao_packet.end                = END;

    t.tv_sec  = 1;
    t.tv_usec = 0;

    bzero(&buff, BUFFLEN);

    sao_packet_net = sao_packet;

    sao_packet_net.syncword     = htons (sao_packet_net.syncword     );
    sao_packet_net.hdr.packetid = htons (sao_packet_net.hdr.packetid );
    sao_packet_net.hdr.pdl      = htons (sao_packet_net.hdr.pdl      );
    sao_packet_net.end          = htons (sao_packet_net.end          );

    while (1)
    {
        if (wait_char(fd, (u_char) 0x41) == 0)
        {
            if (wait_char(fd, (u_char) 0x52) == 0)
            {
                // Ejemplo paquete de entrada desde el serie:
                //'AR_ANG,4932,332.18,DEC_ANG,8191,239.99\r\n'
                sao_packet_net.payload.data[0] = 0x41;
                sao_packet_net.payload.data[1] = 0x52;
                res = soft_read_time_out(t, fd, &sao_packet_net.payload.data[2], ENCOPACKETLEN-2);

                if (res > 0)
                {
                    bzero(tempstring, 6);
                    memcpy(tempstring, &sao_packet_net.payload.data[32], 6);
                    dec = atof(tempstring);
                    bzero(tempstring, 6);
                    memcpy(tempstring, &sao_packet_net.payload.data[12], 6);
                    ha  = atof(tempstring);
                    bzero(tempstring, 6);
                    memcpy(tempstring, &sao_packet_net.payload.data[7], 6);
                    countsha = atoi(tempstring);
                    bzero(tempstring, 6);
                    memcpy(tempstring, &sao_packet_net.payload.data[27], 6);
                    countsdec = atoi(tempstring);

                    dec = decoffset - (dec + (LATITUDE * -1));
                    ha  = (ha - haoffset) * -1;

                    sprintf(datastring, "AR_ANG,%04d,%03.02f,DEC_ANG,%04d,%03.02f\r\n", 
                                                                    countsha, ha, countsdec, dec);

                    memcpy(sao_packet_net.payload.data, datastring, ENCOPACKETLEN);

                    gettimeofday(&sao_packet_net.payload.timestamp, NULL);
                    sao_packet_net.payload.timestamp.tv_sec=htonl(sao_packet_net.payload.timestamp.tv_sec);
                    sao_packet_net.payload.timestamp.tv_usec=htonl(sao_packet_net.payload.timestamp.tv_usec);

                    sao_packet_net.hdr.packet_counter = htons(sao_packet.hdr.packet_counter);
            
                    sendto(socket, &sao_packet_net, sizeof(sao_packet_net), 0, addr, 
                                sizeof(*addr));
            
                    sao_packet.hdr.packet_counter++;
                    if (sao_packet.hdr.packet_counter > 0xFFFF)
                        sao_packet.hdr.packet_counter = 0;
                }
                else
                    perror("Timeout reading serial port.");
            }
        }
    }
}

int main (int argc, char **argv)
{
    int fd, op, port = -1, sock;
    uint16_t packetid = -1;
    char *filename = NULL, *address = NULL;
    struct sockaddr_in addr;
    float decoffset, haoffset;

    while ((op = getopt(argc, argv, "d:n:p:i:a:e:")) != EOF)
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
            case 'i':
                packetid = atoi(argv[optind-1]);
            break;
            case 'e':
                decoffset = atof(argv[optind-1]);
            break;
            case 'a':
                haoffset  = atof(argv[optind-1]);
            break;
            default:
                exit(-1);
        }
    }

    if (address == NULL)
        address = MULTICASTADDR;
    if (port < 0)
        port = MULTICASTPORT;

    sock = open_socket(address, port, &addr);

    network_relay(fd, sock, (struct sockaddr *) &addr, packetid, decoffset, haoffset);

    return 0;
}