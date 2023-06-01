#include <thread>
#include "ConectaCuatro.h"

int main(int argc, char **argv)
{
    ConectaCuatro_Server es(argv[1], argv[2], argv[3]);

    std::cout 
    << "Servidor iniciado \n" 
    << "Esperando a que se conecte un cliente [1/2] \n";
    
    std::thread net_thread([&es](){ es.do_messages(); });
   
    es.input_thread();

    return 0;
}
