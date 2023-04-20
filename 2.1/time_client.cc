// Ignacio Del Castillo Rubio

#include <sys/types.h>
#include <time.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>


int main(int argc, char **argv){
    
    struct addrinfo hints;
    struct addrinfo* server_result;

    if (argc != 4){
        std::cerr << "Invalid arguments\n";
        return -1;
    }

    // Reservar Memoria
    memset(&hints, 0, sizeof(hints));

    // Especificar tipo de socket y filtro de familia
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;

    int adr = getaddrinfo(argv[1], argv[2], &hints, &server_result);
    if(adr != 0){
        std::cerr << gai_strerror(e) << "\n";
        return -1;
    }

    int socketIndex = socket(server_result->ai_family, server_result->ai_socktype, 0);
    if(socketIndex == -1){
        std::cerr << "ERROR : Socket not created" << "\n";
        return -1;
    }

    int buff_size = 80;
    char buff[buff_size];

    int sendBytes = sendto(socketIndex, argv[3], buff_size, 0, server_result->ai_addr, server_result->ai_addrlen);

    struct sockaddr c;
    socklen_t c_l = sizeof(struct sockaddr);

     if(argv[3][0] != 'q'){
        int recvBytes = recvfrom(socketIndex, buff, buff_size, 0, &c, &c_l);
        buff[recvBytes] == '\0';
        std::cout << buff << "\n";
    }

    freeaddrinfo(server_result);
    close(socketIndex);

    return 0;
}