#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/queue.h>
#include <sys/select.h>

typedef enum {FALSE, TRUE} bool;

#define ERROR(err_msg) {perror(err_msg); exit(EXIT_FAILURE);}

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)])) 

uint16_t updates_periodic_interval;           
uint16_t number_of_routers;                   
uint16_t CONTROL_PORT;                        
uint16_t this_router_id;                     
uint16_t this_data_port;                      
uint16_t this_router_port;                   
uint32_t this_router_ip;                      
uint16_t **distance_vector;                   
extern struct timeval timeout;
extern int head_fd;
extern fd_set master_list;
extern fd_set watch_list;
extern char *last_data_packet;
extern char *panultimate_data_packet;

struct router{
    int index;               
    bool connect;            
    uint16_t id;            
    uint16_t router_port;     
    uint16_t data_port;       
    uint32_t ip;             
    uint16_t cost;            
    uint16_t next_hop;        
    LIST_ENTRY(router) next;
};

struct DataConn
{
    int sockfd;
    LIST_ENTRY(DataConn) next;
};


uint8_t global_transfer_id;           
uint8_t global_ttl;                          
uint16_t init_seq_number;            
uint16_t last_seq_number;             
char *buffer;
bool send_finish;

LIST_HEAD(DataConnHead, DataConn) data_list;
LIST_HEAD(router_head, router) router_list;

void print_routing_table();
void print_next_hop();
void print_time_connection();
void print_router_connection();

#endif
