#include <string.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include "../include/global.h"
#include "../include/sendfile_stats_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

void sendfile_stats_response(int sock_index, char *control_payload, uint16_t payload_len){
    uint8_t transfer_id;
    uint16_t padding = 0;
    uint8_t control_code = 6;
    uint8_t response_code = 0;

    uint16_t payload_length = last_seq_number - init_seq_number + 1 + SENDFILE_STATS_HEADER;  
    memcpy(&transfer_id,control_payload,sizeof(uint8_t));
    
    char *sendfile_stats_payload;

    if(transfer_id == global_transfer_id){
        sendfile_stats_payload = (char*)malloc(payload_length);
        memset(sendfile_stats_payload,'\0',payload_length);
   
        uint16_t offset = 0;
        memcpy(sendfile_stats_payload + offset,&global_transfer_id,sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);
        memcpy(sendfile_stats_payload + offset,&global_ttl,sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);
        memcpy(sendfile_stats_payload + offset,&padding,sizeof(uint16_t));
        offset = offset + sizeof(uint16_t);

        uint16_t start = init_seq_number;
        uint16_t end = last_seq_number;
        
        printf("ttl = %d\n",global_ttl); 
        printf("init_seq_number = %d\n",init_seq_number);
        printf("last_seq_number = %d\n",last_seq_number);
        while(start <= end){
            uint16_t seq_number = htons(start);
            memcpy(sendfile_stats_payload + offset,&seq_number,sizeof(uint16_t));
            start++;
            offset = offset + sizeof(uint16_t);
        }

        char *sendfile_stats_response = (char*) malloc(CONTROL_RESPONSE_HEADER_SIZE + payload_length);
        char *sendfile_stats_header = create_response_header(sock_index, control_code, response_code, payload_length);

        memcpy(sendfile_stats_response,sendfile_stats_header,CONTROL_RESPONSE_HEADER_SIZE);
        memcpy(sendfile_stats_response + CONTROL_RESPONSE_HEADER_SIZE,sendfile_stats_payload,payload_length);

        sendALL(sock_index, sendfile_stats_response, CONTROL_RESPONSE_HEADER_SIZE + payload_length);

        free(sendfile_stats_response);
        free(sendfile_stats_payload);
    }
    else{
        char *sendfile_stats_response = (char*) malloc(CONTROL_RESPONSE_HEADER_SIZE);
        char *sendfile_stats_header = create_response_header(sock_index, control_code, response_code, 0);

        memcpy(sendfile_stats_response,sendfile_stats_header,CONTROL_RESPONSE_HEADER_SIZE);

        sendALL(sock_index, sendfile_stats_response, CONTROL_RESPONSE_HEADER_SIZE);
    }
}
