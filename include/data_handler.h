#ifndef _DATA_HANDLER_H_
#define _DATA_HANDLER_H_

#define TRANSFER_ID_OFFSET 4
#define TTL_OFFSET 5
#define SEQ_OFFSET 6
#define FIN_OFFSET 8
#define MAX_FILENAME_LEN 16

int connect_socket;

bool isData(int sock_index);
bool hastid(uint8_t transfer_id);
void update_sendfile_stats_list(uint8_t transfer_id,uint16_t sequencenum,uint32_t next_hop_ip,uint8_t ttl);
uint32_t get_next_hop_ip(uint32_t des_ip);
bool data_recv_file(int sock_index);
int new_data_connection(int data_socket);

#endif
