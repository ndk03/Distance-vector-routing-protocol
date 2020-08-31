#include <string.h>
#include "../include/global.h"
#include "../include/sendfile_manager.h"
#include "../include/penultimate_data_packet_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

void penultimate_data_packet_response(int sock_index){
    uint8_t control_code = 8;
    uint8_t response_code = 0;
    uint16_t payload_length = DATA_PACKET_HEADER_OFFSET + PAYLOAD_LEN;  

    char *penultimate_data_packet_response = (char*)malloc(CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    char *penultimate_data_packet_header = create_response_header(sock_index, control_code, response_code, payload_length);

    memcpy(penultimate_data_packet_response,penultimate_data_packet_header,CONTROL_RESPONSE_HEADER_SIZE);
    memcpy(penultimate_data_packet_response + CONTROL_RESPONSE_HEADER_SIZE,panultimate_data_packet,payload_length);

    sendALL(sock_index, penultimate_data_packet_response, CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    
    free(penultimate_data_packet_response);
}
