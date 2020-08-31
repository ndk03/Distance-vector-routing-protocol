#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/select.h>
#include "../include/init_manager.h"
#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/connection_manager.h"
#include "../include/network_util.h"
#include "../include/time_manager.h"

int create_router_sock(){
    int router_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(router_sock < 0)
        ERROR("router_socket() failed");

    struct sockaddr_in router_addr;
    socklen_t addrlen = sizeof(router_addr);
    bzero(&router_addr, sizeof(router_addr));
    router_addr.sin_family = AF_INET;
    router_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    router_addr.sin_port = this_router_port;

    int yes = 1;
    if(setsockopt(router_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
        ERROR("setsockopt() failed");

    if(bind(router_sock, (struct sockaddr *)&router_addr, sizeof(router_addr)) < 0)
        ERROR("bind() failed");
    return router_sock;
}

int create_data_socket(){
    int data_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(data_sock < 0)
        ERROR("socket() failed");

    struct sockaddr_in data_addr;
    socklen_t addrlen = sizeof(data_addr);

    /* Make socket re-usable */
    if(setsockopt(data_sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
        ERROR("setsockopt() failed");

    bzero(&data_addr, sizeof(data_addr));

    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    data_addr.sin_port = this_data_port;

    if(bind(data_sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0)
        ERROR("bind() failed");

    if(listen(data_sock, number_of_routers) < 0)
        ERROR("listen() failed");

    return data_sock;
}

void init_router_list(char *cntrl_payload, uint16_t offset, uint16_t payload_len){
    int index, i;
    uint16_t *id = (uint16_t*)malloc(sizeof(uint16_t)*number_of_routers);
    uint16_t *cost = (uint16_t*)malloc(sizeof(uint16_t)*number_of_routers);
    LIST_INIT(&router_list);
    for(index = 0;index < number_of_routers;++index){
        struct router *r = (struct router*)malloc(sizeof(struct router));

        r->index = index;
        memcpy(&r->id, cntrl_payload + offset, sizeof(r->id));
        memcpy(&id[index],cntrl_payload + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);

        memcpy(&r->router_port, cntrl_payload + offset, sizeof(r->router_port));
        offset += sizeof(uint16_t);

        memcpy(&r->data_port, cntrl_payload + offset, sizeof(r->data_port));
        offset += sizeof(uint16_t);

        memcpy(&r->cost, cntrl_payload + offset, sizeof(uint16_t));
        memcpy(&cost[index],cntrl_payload + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        if(ntohs(cost[index]) == UINT16_MAX){
            r->next_hop = htons(UINT16_MAX);
            r->connect = FALSE;
        }
        else if(ntohs(cost[index]) == 0){
            r->next_hop = r->id;
            r->connect = FALSE;
            this_router_id = index;
        }
        else{
            r->next_hop = r->id;
            r->connect = TRUE;
        }

        memcpy(&r->ip, cntrl_payload + offset, sizeof(r->ip));
        offset += sizeof(uint32_t);

        LIST_INSERT_HEAD(&router_list, r, next);
    }
    
    /* Init distance vector, store as host */
    for(i = 0;i != number_of_routers;++i){
        distance_vector[this_router_id][i] = ntohs(cost[i]);
    }
    free(id);
    free(cost);
}

/* Set all element of distanve vector to UINT16_MAX */
void init_distance_vector(){
    int i, j;
    distance_vector = (uint16_t**)malloc(number_of_routers*number_of_routers*sizeof(uint16_t*));
    for(i = 0;i != number_of_routers;++i){
        distance_vector[i] = (uint16_t*)malloc(number_of_routers*sizeof(uint16_t));
        for(j = 0;j != number_of_routers;++j)
            distance_vector[i][j] = UINT16_MAX;
    }
}

/* Init this_router_ip, this router_port, this_data_port */
void init_this_ip_port(){
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        if(ntohs(r->cost) == 0){
            this_router_port = r->router_port;
            this_data_port = r->data_port;
            this_router_ip = r->ip;
        }
    }
}


/* Init_response */
void init_response(int sock_index, char *cntrl_payload, uint16_t payload_len){
    /* Init number_of_routers, in this project is 5, store as host */
    uint16_t offset = 0;
    memcpy(&number_of_routers, cntrl_payload+offset, sizeof(number_of_routers));
    number_of_routers = ntohs(number_of_routers);
    offset += sizeof(number_of_routers);
    
    /* Init update interval, store as host */
    memcpy(&updates_periodic_interval, cntrl_payload+offset, sizeof(updates_periodic_interval));
    offset += sizeof(updates_periodic_interval);
    updates_periodic_interval = ntohs(updates_periodic_interval);

    /* Init timeout */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    /* Init related varilables */
    init_distance_vector();
    init_router_list(cntrl_payload, offset, payload_len);
    init_this_ip_port();
    init_time();
    LIST_INIT(&data_list);

    /* Create data and router socket */
    router_socket = create_router_sock();
    data_socket = create_data_socket();
    FD_SET(router_socket, &master_list);
    FD_SET(data_socket, &master_list);
    int hf = router_socket > data_socket ? router_socket : data_socket;
    head_fd = head_fd > hf ? head_fd : hf;

    /* Send response command */
    uint8_t control_code = 1;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;
    char *cntrl_header = create_response_header(sock_index, control_code, response_code, payload_length);
    char *init_response = (char*) malloc(CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    memcpy(init_response,cntrl_header,CONTROL_RESPONSE_HEADER_SIZE);
    sendALL(sock_index, init_response, CONTROL_RESPONSE_HEADER_SIZE + payload_length);
    free(init_response);
}



