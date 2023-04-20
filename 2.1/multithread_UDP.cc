#include <iostream>
#include <sys/socket.h>
#include <string.h>

#include <errno.h>

#include <sys/types.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>

#include <string>

#define THREAD_MAX 5

void process_message(int socket_descriptor)
{
	char buffer[2000];
    char server_result[NI_MAXSERV];
    
    char host[NI_MAXHOST];

    struct sockaddr_storage c;
    socklen_t  c_len = sizeof(struct sockaddr_storage);

    ssize_t bytes_received = recvfrom(socket_descriptor, buffer, 1499, 0, (struct sockaddr*) &c, &c_len);
    buffer[bytes_received] = '\0';


    getnameinfo((struct sockaddr*) &c, c_len, host, NI_MAXHOST, server_result, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    std::cout << "ip = " << host << " ( " << server_result << " ) \n";

    std::cout << "\t Message = " << buffer << " " << std::this_thread::get_id();

    sleep(3);
    sendto(socket_descriptor, (void*) buffer, bytes_received, 0, (struct sockaddr*) &c, c_len);
}


int main(int argc, char *argv[]){

	struct addrinfo address_hints;
	struct addrinfo *address_results;


	memset(&address_hints, 0, sizeof(struct addrinfo));
    
	address_hints.ai_flags = AI_PASSIVE;
	address_hints.ai_family = AF_INET; //ipv4
	address_hints.ai_socktype = SOCK_DGRAM;

	int result_code = int getaddrinfo(argv[1], argv[2], &address_hints, &address_results);

	int socket_descriptor = socket(address_results->ai_family, address_results->ai_socktype, address_results->ai_protocol);
	result_code = bind(socket_descriptor, address_results->ai_addr, address_results->ai_addrlen);
	freeaddrinfo(address_results);

	std::vector<std::thread> allThreadsVector;
	for(int i = 0; i < THREAD_MAX; i++){
		allThreadsVector.push_back(std::thread([socket_descriptor](){
			while(true)
				process_message(socket_descriptor);}));
	}
	for(auto &thread: allThreadsVector)
		thread.join();


	return 0;
}