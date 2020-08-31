#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include "../include/global.h"
#include "../include/sendfile_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

int connect_destination(uint32_t des_ip){
    uint16_t des_port;
    struct router *rout;
    
    LIST_FOREACH(rout,&router_list,next){
        if(rout->ip == des_ip){
            des_port = rout->data_port;
        }
    }

    int connect_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in des_addr; 
    bzero(&des_addr,sizeof(des_addr));
    des_addr.sin_family = AF_INET;
    des_addr.sin_port = des_port;
    des_addr.sin_addr.s_addr = des_ip;
    if ((connect(connect_socket, (struct sockaddr *)&des_addr, sizeof(struct sockaddr))) < 0){
        close(connect_socket);
        ERROR("Connect error.");
    }
    return connect_socket;
}

void sendfile_response(int sock_index, char *cntrl_payload, uint16_t payload_len){
    uint32_t des_ip;
    uint8_t ttl;
    uint8_t transfer_id;
    uint16_t seq_number;
    uint8_t offset = 0;
    char file_name[16];
    memset(file_name,'\0',16);

    memcpy(&des_ip, cntrl_payload + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&ttl, cntrl_payload + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&transfer_id, cntrl_payload + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&seq_number, cntrl_payload + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(file_name, cntrl_payload + offset, payload_len-8);

    global_transfer_id = transfer_id;
    global_ttl = ttl;
    init_seq_number = ntohs(seq_number);
    last_seq_number = ntohs(seq_number);

    uint16_t next_hop;
    uint32_t next_hop_ip;
    struct router *rout;
    LIST_FOREACH(rout,&router_list,next){
        if(rout->ip == des_ip){
            next_hop = rout->next_hop;
        }
    }
    LIST_FOREACH(rout,&router_list,next){
        if(rout->id == next_hop){
            next_hop_ip = rout->ip;
        }
    }
    int connect_socket = connect_destination(next_hop_ip);

    FILE *stream = fopen(file_name,"r");
    fseek(stream, 0L, SEEK_END);
    int sz = ftell(stream);
    int count = 0;
    fseek(stream, 0L, SEEK_SET);
    char *total_payload = (char*)malloc(sz);
    memset(total_payload,0,sz);
    fread(total_payload,sz,1,stream);
    fclose(stream);

    while(count * PAYLOAD_LEN < sz){
        char *header = (char*)malloc(DATA_PACKET_HEADER_OFFSET);
        uint16_t offset = 0;
        memcpy(header+offset,&des_ip,sizeof(uint32_t));
        offset = offset + sizeof(uint32_t);
        memcpy(header+offset,&transfer_id,sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);
        memcpy(header+offset,&ttl,sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);

        uint16_t nseqnum = htons(last_seq_number);
        memcpy(header+offset,&nseqnum,sizeof(uint16_t));
        offset = offset + sizeof(uint16_t);

        if((count+1) * PAYLOAD_LEN >= sz){
            uint8_t fin_one = 1<<7;
            memcpy(header+offset,&fin_one,sizeof(uint8_t));
            offset = offset + sizeof(uint8_t);
        }
        else{
            uint8_t fin_zero = 0;
            memcpy(header+offset,&fin_zero,sizeof(uint8_t));
            offset = offset + sizeof(uint8_t);
        }

        uint8_t padding8 = 0;
        uint16_t padding16 = 0;
        memcpy(header+offset,&padding8,sizeof(uint8_t));
        memcpy(header+offset,&padding16,sizeof(uint16_t));

        sendALL(connect_socket,header,DATA_PACKET_HEADER_OFFSET);
        sendALL(connect_socket,total_payload + count*PAYLOAD_LEN,PAYLOAD_LEN);

        last_seq_number = init_seq_number + count;
        count++;
        free(header);
    }
    close(connect_socket);

    uint8_t control_code = 5;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;  
    char *sendfile_response = (char*) malloc(CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    char *sendfile_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(sendfile_response,sendfile_header,CONTROL_RESPONSE_HEADER_SIZE);
    sendALL(sock_index, sendfile_response, CONTROL_RESPONSE_HEADER_SIZE + payload_length);

    free(total_payload);
    free(sendfile_response);
}
