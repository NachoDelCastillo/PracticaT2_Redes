#include <iostream>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char **argv){
    
    struct addrinfo hints;
    struct addrinfo* result;
    if (argc != 3){
        std::cerr << "Invalid arguments\n";
        return -1;
    }

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int addr = getaddrinfo(argv[1], argv[2], &hints, &result);
    if(addr < 0){
        std::cerr << gai_strerror(addr) << "\n";
        return -1;
    }

    int sockIndex = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    connect(sockIndex,result->ai_addr, result->ai_addrlen);

    //Obtiene informacion del buff y la gestiona
    int buff_num = 1024;
    char buff[buff_num];

    bool stopLoop = false;
    while(!stopLoop){

        std::cin.getline(buff, buff_num);


        if(strcmp(buff, "Q") == 0){
            stopLoop = true;
            break;
        }

        send(sockIndex, &buff, buff_num, 0);
        recv(sockIndex, &buff, buff_num, 0);

        std::cout<<buff<<"\n";
    }

    
    freeaddrinfo(result);

    
    close(sockIndex);
    return 0;

}