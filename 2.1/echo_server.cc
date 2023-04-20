// Ignacio Del Castillo Rubio
#include <sys/types.h>
#include <netdb.h>
#include <string>

#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>

#include <errno.h>
#include <unistd.h>

int main(int argc, char *argv[]){

    struct addrinfo *result;
    struct addrinfo hints;

	// Reservar memoria
    memset(&hints, 0, sizeof(struct addrinfo));

	// Especificar filtros y tipo de socket
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    int rc = getaddrinfo(argv[1], argv[2], &hints, &result);

    if(rc!=0){
        std::cerr << "ERROR =  " << gai_strerror(rc) << "\n";
        return -1;
    }

	// Acceder al socket
    int sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	// Enlazarlo
    rc = bind(sd, result->ai_addr, result->ai_addrlen);

    if(rc==-1){
        std::cerr << "[bind]: " << strerror(errno) << "\n";
        return -1;
    }

    freeaddrinfo(result);
    listen(sd,16);
    while(true){

        char serv[NI_MAXSERV];
		
        char host[NI_MAXHOST];


        struct sockaddr client;
        socklen_t  clientlen = sizeof(struct sockaddr);
        int client_sd = accept(sd, (struct sockaddr*)&client, &clientlen);
        getnameinfo((struct sockaddr*)&client, clientlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        std::cout << "ip = " << host << " [: " << serv << "]\n";

                while(true){
            char buffer[1500];
            ssize_t bytes = recv(client_sd, buffer, 1499, 0);
            if(bytes==0){
                std::cout<<"FIN DE CONEXION\n";
                break;
            }

                        buffer[bytes] = '\0';
            std::cout << "\tMSG: " << buffer;
            send(client_sd, buffer, bytes, 0);
        }
        close(client_sd);
    }

        close(sd);
    return 0;
}