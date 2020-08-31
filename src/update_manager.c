#include <string.h>
#include <netinet/in.h>
#include <sys/queue.h>
#include <sys/time.h>

#include "../include/global.h"
#include "../include/update_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/time_manager.h"

void update_response(int sock_index, char *control_payload){
    uint16_t id;
    uint16_t cost;
    uint16_t offset = 2;
    uint8_t control_code = 3;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;
    int index = -1;
    struct router *rout;
    struct time *time;

    memcpy(&id, control_payload, sizeof(uint16_t));
    memcpy(&cost, control_payload + offset, sizeof(uint16_t));
    
    LIST_FOREACH(rout,&router_list,next){
        if(id == rout->id){
            rout->cost = cost;
            rout->connect = TRUE; 
            index = rout->index;
        }
    }
    
    LIST_FOREACH(time,&time_list,next){
        if(time->index == index){
            gettimeofday(&time->begin_send_time,NULL);
            gettimeofday(&time->begin_expire_time,NULL);
            time->send_time.tv_sec = updates_periodic_interval;
            time->send_time.tv_usec = 0;
            time->expire_time.tv_sec = 3 * updates_periodic_interval;
            time->expire_time.tv_usec = 0;
            time->isconnect = TRUE;
        }
    }
    printf("---------------------------update-----------------------------\n");
    print_time_connection();
    print_router_connection();
    printf("--------------------------------------------------------------\n");

    
    char *update_response = (char*) malloc(CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    char *update_header = create_response_header(sock_index, control_code, response_code, payload_length);

    memcpy(update_response,update_header,CONTROL_RESPONSE_HEADER_SIZE);

    sendALL(sock_index, update_response, CONTROL_RESPONSE_HEADER_SIZE + payload_length);

    free(update_response);
}
