#include <sys/queue.h>
#include <string.h>
#include <netinet/in.h>
#include "../include/global.h"
#include "../include/routing_table_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

void routing_table_response(int sock_index){
    uint8_t control_code = 2;
    uint8_t response_code = 0;
    uint16_t payload_length = 4 * sizeof(uint16_t) * number_of_routers;

    char *routing_table_response = (char*) malloc(CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    char *cntrl_header = create_response_header(sock_index, control_code, response_code, payload_length);

    memcpy(routing_table_response,cntrl_header,CONTROL_RESPONSE_HEADER_SIZE);

    struct router *rout;
    uint16_t offset = 0;
    uint16_t padding = 0;

    LIST_FOREACH(rout,&router_list,next){
        memcpy(routing_table_response + CONTROL_RESPONSE_HEADER_SIZE + offset,&rout->id,sizeof(uint16_t));
        offset = offset + 2;
        memcpy(routing_table_response + CONTROL_RESPONSE_HEADER_SIZE + offset, &padding, sizeof(uint16_t));
        offset = offset + 2;
        memcpy(routing_table_response + CONTROL_RESPONSE_HEADER_SIZE + offset, &rout->next_hop, sizeof(uint16_t));
        offset = offset + 2;

        uint16_t cost = htons(distance_vector[this_router_id][rout->index]);  
        memcpy(routing_table_response + CONTROL_RESPONSE_HEADER_SIZE + offset, &cost, sizeof(uint16_t));
        offset = offset + 2;
    }

    sendALL(sock_index, routing_table_response, CONTROL_RESPONSE_HEADER_SIZE + payload_length);

    free(routing_table_response);
}

