#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "../include/global.h"
#include "../include/router_handler.h"
#include "../include/time_manager.h"

uint16_t min(int a,int b){
    int result = a > b ? b : a;
    return result >= UINT16_MAX ? UINT16_MAX : result;
}

void recv_update(int router_socket){
    struct sockaddr_in from_addr;    
    int numbytes = 0;
    int length = UPDATE_HEADER + number_of_routers * UPDATE_ENTRY_LENGTH;
    char *buffer = (char*)malloc(length);
    int addr_len = sizeof(struct sockaddr);
    if ((numbytes = recvfrom(router_socket,buffer,length,0,(struct sockaddr *)&from_addr,&addr_len)) < 0){
        ERROR("Recvfrom error.");
        exit(1);
    }
 
    uint16_t number_of_update_fields;  
    uint16_t source_router_port;       
    uint32_t source_ip_addr;           
    uint16_t offset = 0;
    int i, j;

    memcpy(&number_of_update_fields, buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(&source_router_port, buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(&source_ip_addr, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    uint16_t num = ntohs(number_of_update_fields);
    uint32_t *receive_ip = (uint32_t*)malloc(num * sizeof(uint32_t));
    uint16_t *receive_port = (uint16_t*)malloc(num * sizeof(uint16_t));
    uint16_t *receive_id = (uint16_t*)malloc(num * sizeof(uint16_t));
    uint16_t *receive_cost = (uint16_t*)malloc(num * sizeof(uint16_t));

    for(i = 0;i != num;++i){
        memcpy(&receive_ip[i],buffer + offset,sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(&receive_port[i],buffer + offset,sizeof(uint16_t));
        offset += sizeof(uint16_t);
        offset += sizeof(uint16_t);    //padding
        memcpy(&receive_id[i],buffer + offset,sizeof(uint16_t));
        offset += sizeof(uint16_t);
        memcpy(&receive_cost[i],buffer + offset,sizeof(uint16_t));
        offset += sizeof(uint16_t);
    }
    
    struct router *rout;
    LIST_FOREACH(rout,&router_list,next){
        if(rout->ip == source_ip_addr){
            for(i = 0;i != num;++i){
                struct router *r;
                LIST_FOREACH(r,&router_list,next){
                    if(receive_id[i] == r->id){
                        distance_vector[rout->index][r->index] = ntohs(receive_cost[i]);
                        continue;
                    }
                }
            }
        }
    }
    
    for(i = 0;i != num;++i){
        if(i == this_router_id)
            continue;
        uint16_t newcost;
        struct router *r;
        LIST_FOREACH(r,&router_list,next){
            if(r->index == i){
                if(r->connect == TRUE){
                    r->next_hop = r->id;
                    newcost = ntohs(r->cost);
                }
                else{
                    r->next_hop = htons(UINT16_MAX);
                    newcost = ntohs(UINT16_MAX);
                }
            }
        }
        for(j = 0;j != num;++j){
            if(j == this_router_id){
                continue;
            }

            bool flag = FALSE;
            struct time *t;
            LIST_FOREACH(t,&time_list,next){
                if(t->index == j){
                    if(t->isconnect == FALSE){
                        printf("error.\n");
                        flag = TRUE;
                    }
                }
            }
            if(flag == TRUE)
                continue;

            uint16_t origcost = newcost;
            uint16_t prevcost;
            struct router *temp_router;
            LIST_FOREACH(temp_router,&router_list,next){
                if(temp_router->index == j)
                    prevcost = ntohs(temp_router->cost);
            }

            newcost = min((int)newcost,prevcost+(int)distance_vector[j][i]);
            if(origcost != newcost){
                struct router *r2;
                LIST_FOREACH(r2,&router_list,next){
                    if(r2->index == i){
                         struct router *r3;
                         LIST_FOREACH(r3,&router_list,next){
                             if(r3->index == j){
                                 r2->next_hop = r3->id;
                             }
                         }
                    }
                }
            }
        }
        distance_vector[this_router_id][i] = newcost;
    }

    struct time *t;
    LIST_FOREACH(t,&time_list,next){
        if(t->isconnect == TRUE && t->router_ip_addr == source_ip_addr){
            printf("Update expire time of router %d\n",t->index);
            gettimeofday(&t->begin_expire_time,NULL);
            t->expire_time.tv_sec = 3 * updates_periodic_interval;
            t->expire_time.tv_usec = 0;
        }
    }

    print_routing_table();
    print_next_hop();
}

void send_vector(int router_socket, struct router *r){   
    struct sockaddr_in des_addr;
    des_addr.sin_family = AF_INET;
    des_addr.sin_port = r->router_port;
    des_addr.sin_addr.s_addr = r->ip;
    bzero(&(des_addr.sin_zero), 8);

    int numbytes = 0;
    uint16_t offset = 0;
    int length = UPDATE_HEADER + number_of_routers * UPDATE_ENTRY_LENGTH;
    char *buffer = (char*)malloc(length);

    uint16_t number_of_update_fields = htons(number_of_routers);
    uint16_t source_router_port = this_router_port;
    uint32_t source_ip_addr = this_router_ip;
    uint16_t padding = 0;

    memcpy(buffer+offset,&number_of_update_fields,sizeof(uint16_t));
    offset += UPDATE_FIELD;
    memcpy(buffer+offset,&source_router_port,sizeof(uint16_t));
    offset += SOURCE_ROUTER_PORT;
    memcpy(buffer+offset,&source_ip_addr,sizeof(uint32_t));
    offset += SOURCE_IP_ADDR;

    struct router *rout;
    LIST_FOREACH(rout,&router_list,next){
        memcpy(buffer+offset,&rout->ip,sizeof(uint32_t));
        offset += sizeof(uint32_t);

        memcpy(buffer+offset,&rout->router_port,sizeof(uint16_t));
        offset += sizeof(uint16_t);

        memcpy(buffer+offset,&padding,sizeof(uint16_t));
        offset += sizeof(uint16_t);

        memcpy(buffer+offset,&rout->id,sizeof(uint16_t));
        offset += sizeof(uint16_t);

        uint16_t cost = htons(distance_vector[this_router_id][rout->index]);
        memcpy(buffer+offset,&cost,sizeof(uint16_t));
        offset += sizeof(uint16_t);
    }

    printf("send %d buffer\n",ntohs(r->id));
    if ((numbytes = sendto(router_socket,buffer,length,0,(struct sockaddr *)&des_addr,sizeof(struct sockaddr_in))) < 0) {
        ERROR("Send to error");
    }
}
