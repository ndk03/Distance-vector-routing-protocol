#include <sys/select.h>
#include <netinet/in.h>

#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/router_handler.h"
#include "../include/data_handler.h"
#include "../include/time_manager.h"
#include "../include/sendfile_manager.h"

fd_set watch_list, master_list;
int head_fd;
struct timeval timeout;
char *last_data_packet;
char *panultimate_data_packet;
bool send_finish;

void main_loop()
{
    int selret, sock_index, fdaccept;
    timeout.tv_sec = 1000000;
    while(TRUE){
        watch_list = master_list;
        selret = select(head_fd+1, &watch_list, NULL, NULL, &timeout);

        if(selret < 0)
            ERROR("select failed.");

        if(selret > 0){
            for(sock_index=0; sock_index<=head_fd; sock_index+=1){

                if(FD_ISSET(sock_index, &watch_list)){

                    if(sock_index == control_socket){
                        fdaccept = new_control_conn(sock_index);

                        FD_SET(fdaccept, &master_list);
                        if(fdaccept > head_fd) head_fd = fdaccept;
                    }

                    else if(sock_index == router_socket){
                       
                        recv_update(router_socket);
                    }

                  
                    else if(sock_index == data_socket){
                       
                        int newfd = new_data_connection(data_socket);
                        head_fd = newfd > head_fd ? newfd : head_fd;
                        FD_SET(newfd, &master_list);
                    }

                    else{
                        if(isControl(sock_index)){
                            if(!control_recv_hook(sock_index)) 
                                FD_CLR(sock_index, &master_list);
                        }
                        else if(isData(sock_index)){
                            if(!data_recv_file(sock_index))
                                FD_CLR(sock_index, &master_list);
                        }
                        else ERROR("Unknown socket index");
                    }
                }
            }
        }
        
        if(selret == 0){
            update_time(router_socket);
            timeout = time_out();
            printf("Timeout:tv_sec = %ld,tv_usec = %ld\n",timeout.tv_sec,timeout.tv_usec);
        }
    }
}

void init()
{
    send_finish = TRUE;
    control_socket = create_control_sock();

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    FD_SET(control_socket, &master_list);
    head_fd = control_socket;

    last_data_packet = (char*)malloc(PAYLOAD_LEN + DATA_PACKET_HEADER_OFFSET);
    panultimate_data_packet = (char*)malloc(PAYLOAD_LEN + DATA_PACKET_HEADER_OFFSET);
    main_loop();
}
