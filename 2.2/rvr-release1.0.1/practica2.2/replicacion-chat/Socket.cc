#include <string.h>

#include "Serializable.h"
#include "Socket.h"

Socket::Socket(const char* address, const char* port) : sd(-1)
{
    // Construir un socket de tipo AF_INET y SOCK_DGRAM usando getaddrinfo.
    struct addrinfo hints;
    struct addrinfo* result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int error = getaddrinfo(address, port, &hints, &result);
    if (error != 0) {
        std::cerr << gai_strerror(error) << "\n";
        throw("Error socket");
    }

    int sock = socket(result->ai_family, result->ai_socktype, 0);
    if (sock == -1) {
        std::cerr << "Error creating socket" << "\n";
        throw("Error socket");
    }
    // Con el resultado inicializar los miembros sd, sa y sa_len de la clase
    sd = sock;
    sa = (*result->ai_addr);
    sa_len = result->ai_addrlen;
}

int Socket::recv(Serializable& obj, Socket*& sock)
{
    struct sockaddr address;
    socklen_t address_len = sizeof(struct sockaddr);

    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = ::recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &address, &address_len);

    if (bytes <= 0) {
        return -1;
    }

    if (sock != nullptr) {
        sock = new Socket(&address, address_len);
    }

    obj.from_bin(buffer);

    return 0;
}

int Socket::send(Serializable& obj, const Socket& sock)
{
    // Serializar el objeto
    obj.to_bin();
    // Enviar el objeto binario a sock usando el socket sd
    sendto(sd, obj.data(), obj.size(), 0, &sock.sa, sock.sa_len);
}

bool operator==(const Socket& s1, const Socket& s2)
{
    // Comparar los campos sin_family, sin_addr.s_addr y sin_port
    // de la estructura sockaddr_in de los Sockets s1 y s2
    // Retornar false si alguno difiere
    struct sockaddr_in* info1 = (struct sockaddr_in*)&s1.sa;
    struct sockaddr_in* info2 = (struct sockaddr_in*)&s2.sa;
    if (info1->sin_family != info2->sin_family ||
        info1->sin_addr.s_addr != info2->sin_addr.s_addr ||
        info1->sin_port != info2->sin_port)
        return false;
    else
        return true;
};

std::ostream& operator<<(std::ostream& os, const Socket& s)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    getnameinfo((struct sockaddr*)&(s.sa), s.sa_len, host, NI_MAXHOST, serv,
                NI_MAXSERV, NI_NUMERICHOST);

    os << host << ":" << serv;

    return os;
};
