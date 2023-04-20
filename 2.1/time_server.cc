#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <unistd.h>

int main(int argc, char *argv[]){



	struct addrinfo *result;
    struct addrinfo hints;


	memset(&hints, 0, sizeof(struct addrinfo));

    // Especificar filtros
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET; //ipv4

    // Acceder a la info
	int rc = getaddrinfo(argv[1], argv[2], &hints, &result);
    // Control de error
	if(rc!=0){
		std::cerr << "[addrinfo]: " << gai_strerror(rc) << "\n";
		return -1;
	}

    // Acceder al numero de socket
    int sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    // Y enlazarlo
	rc = bind(sd, result->ai_addr, result->ai_addrlen);


	if(rc == -1){
		std::cerr << "[BIND]: " << strerror(errno) << "\n";
		return -1;
	}


	freeaddrinfo(result);

    while(true){
        
		char serv[NI_MAXSERV];
		char host[NI_MAXHOST];

		char buff[1500];


		struct sockaddr client;
		socklen_t  clientlen = sizeof(struct sockaddr);
		ssize_t bytes = recvfrom(sd, buff, 1499, 0, (struct sockaddr*)&client, &clientlen);



        buff[bytes] = '\0';
        // Acceder a informacion
		getnameinfo((struct sockaddr*)&client, clientlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        std::cout << "ip = " << host << " puerto = " << serv << " . \n";

		if(buff[0]=='Q'){
			std::cout<<"QUIT\n";
			break;
		}

        time_t t = time(NULL);
		struct tm* time = localtime(&t);
		int timeSize=0;
		char bufferTime[1750];


        if(buff[0]=='d')
			timeSize=strftime(bufferTime, 1750, "%D", time);
		else if(buff[0]=='t')
			timeSize=strftime(bufferTime, 1750, "%R", time);
        
		sendto(sd, (void*) bufferTime, timeSize, 0, (struct sockaddr*)&client, clientlen);
	}

	close(sd);
    
	return 0;
}
