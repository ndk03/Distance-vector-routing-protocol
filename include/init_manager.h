#ifndef _INIT_MANAGER_H_
#define _INIT_MANAGER_H_

int create_router_sock();
int create_data_socket();
void init_router_list(char *cntrl_payload, uint16_t offset, uint16_t payload_len);
void init_distance_vector();
void init_this_ip_port();
void init_response(int sock_index, char *cntrl_payload, uint16_t payload_len);

#endif
