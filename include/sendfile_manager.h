#ifndef _SENDFILE_MANAGER_H_
#define _SENDFILE_MANAGER_H_

#define PAYLOAD_LEN 1024
#define DATA_PACKET_HEADER_OFFSET 12
#define FIN_ONE 1<<7
#define FIN_ZERO 0

int connect_destination(uint32_t des_ip);
char *create_packet_header(uint32_t des_ip,uint8_t transfer_id,uint8_t ttl,uint16_t seqnum,uint8_t fin);
void sendfile_response(int sock_index, char *cntrl_payload, uint16_t payload_len);

#endif
