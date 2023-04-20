#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <string.h>
#include <thread>

#include <iostream>
#include <sys/types.h>


class MessageProcessor
{
public:
    MessageProcessor(int sdClient){
        sd = sdClient;
    }

    void ProcessMessage(){


        while(true){
            const int buffer_size = 80;


            char buffer[buffer_size];
            int received_bytes = recv(sd, buffer, buffer_size - 1, 0);

            buffer[received_bytes] = '\0';

            std::cout << "\t THREAD ID = " << std::this_thread::get_id() << " / BUFFER = " << buffer << "\n";
            send(sd, buffer, received_bytes, 0);
        }
        close(sd);
    }

private:
    int sd;
};


int main(int argc, char **argv){
    
    struct addrinfo hints;
    struct addrinfo* result;

    if (argc != 3){
        std::cerr << "Invalid arguments\n";
        return -1;
    }

    memset(&hints, 0, sizeof(hints));

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int socket_fd = socket(result->ai_family, result->ai_socktype, 0);
    if(socket_fd == -1){
        std::cerr << "Error creating socket" << "\n";
        return -1;
    }



    int error = getaddrinfo(argv[1], argv[2], &hints, &result);
    if(error != 0){
        std::cerr << gai_strerror(error) << "\n";
        return -1;
    }

    bind(socket_fd, result->ai_addr, result->ai_addrlen);

    listen(socket_fd, 16);
    freeaddrinfo(result);


    while(true){
        char host[NI_MAXHOST];
        char service[NI_MAXSERV];


        struct sockaddr client_addr;
        socklen_t client_addr_len = sizeof(struct sockaddr);
        int client_fd = accept(socket_fd, &client_addr, &client_addr_len);

        getnameinfo(&client_addr, client_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICHOST);
        std::cout << "HOST = " << host << "   PUERTO = " << service << "\n";

        //Create and detach thread
        std::thread([&](){
            MessageProcessor processor(client_fd);
            processor.ProcessMessage();
        }).detach();
    }

    close(socket_fd);

    return 0;
}
