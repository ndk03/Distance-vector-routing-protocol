#include <string.h>
#include "../include/global.h"
#include "../include/sendfile_manager.h"
#include "../include/last_data_packet_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

void last_data_packet_response(int sock_index){
    uint8_t control_code = 7;
    uint8_t response_code = 0;
    uint16_t payload_length = DATA_PACKET_HEADER_OFFSET + PAYLOAD_LEN;  
    char *last_data_packet_response = (char*)malloc(CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    char *last_data_packet_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(last_data_packet_response,last_data_packet_header,CONTROL_RESPONSE_HEADER_SIZE);
    memcpy(last_data_packet_response + CONTROL_RESPONSE_HEADER_SIZE,last_data_packet,payload_length);
    sendALL(sock_index, last_data_packet_response, CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    free(last_data_packet_response);
}
