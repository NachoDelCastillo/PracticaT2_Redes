// Ignacio Del Castillo Rubio
#include <iostream>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>

#include <sys/socket.h>

int main(int argc, char **argv){
    


    struct addrinfo* result;
    struct addrinfo hints;

    // Comprobar si hay suficientes argumentos
    if (argc != 2){
        std::cerr << " Invalid arguments \n";
        return -1;
    }

    // Reservar memoria 
    memset(&hints, 0, sizeof(hints));

    // Especificar filtros
    hints.ai_family = AF_UNSPEC;

    // Acceder a la informacion
    int e = getaddrinfo(argv[1], NULL, &hints, &result);

    // Devolver un error si no es valido
    if(e != 0){
        std::cerr << gai_strerror(e) << "\n";
        return -1;
    }


    struct addrinfo* it;
    for(it = result; it != NULL; it = it->ai_next){
        char serv[NI_MAXSERV];
        char host[NI_MAXHOST];

        getnameinfo(it->ai_addr, it->ai_addrlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST);

        std::cout << "HOST "  << host << "  /  FAMILY "  << it->ai_family
            << "  /  SOCKET "  << it->ai_socktype << "\n";
    }




    freeaddrinfo(result);
    return 0;
}


