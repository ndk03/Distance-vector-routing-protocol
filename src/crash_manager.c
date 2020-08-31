#include <string.h>
#include <unistd.h>
#include "../include/global.h"
#include "../include/crash_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/connection_manager.h"

void crash_response(int sock_index){
    uint8_t control_code = 4;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;
    char *crash_response = (char*) malloc(CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    char *crash_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(crash_response,crash_header,CONTROL_RESPONSE_HEADER_SIZE);
    sendALL(sock_index, crash_response, CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    free(crash_response);
    close(router_socket);
    close(data_socket);
}
