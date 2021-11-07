#define SYNCWORD       0xA185
#define END            0x581A
#define VERSION        0x1
#define ENCOID         0x0001
#define CMDPACKET      0x8E
#define REPORTPACKET   0x8F

struct __attribute__((__packed__)) SAO_data_transport_header 
{
    uint16_t    syncword;
    uint8_t     version;
    uint16_t    packetid;
    uint8_t     message_type;
    uint16_t    packet_counter;
    uint16_t    pdl;
};

struct __attribute__((__packed__)) SAO_data_transport
{
    struct SAO_data_transport_header *hdr;
    uint8_t                          *data;
    uint16_t                         end;
};