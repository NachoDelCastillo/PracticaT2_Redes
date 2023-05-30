#include "ConectaCuatro.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ConectaCuatro_Message::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data
    char *tmp = _data;

    // Un index para identificar el tipo de mensaje 
    memcpy(tmp, &type, sizeof(uint8_t));
    tmp +=sizeof(uint8_t);

    // Espacio para el nombre
    memcpy(tmp, nick.c_str(), sizeof(char) * 8);
    tmp += 8*sizeof(char);

    // Importante que el tamaño del mensaje pueda almacenar 
    // el estado del tablero entero, invluyendo los "|" 
    memcpy(tmp, message.c_str(), sizeof(char) * 300); // 80
}

int ConectaCuatro_Message::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);


    //Reconstruir la clase usando el buffer _data
    char *tmp = bobj;
    
    memcpy(&type, tmp, sizeof(uint8_t));
    tmp +=sizeof(uint8_t);

    nick = tmp;
    tmp += sizeof(char) * 8;

    message = tmp;

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void GameServer::do_messages()
{
    while (true)
    {
            // Recibir mensajes del cliente conectado
            ConectaCuatro_Message msg;
            Socket *client = new Socket(socket);
            socket.recv(msg,client);



                            // Si ya se ha registrado un cliente, no permitir el Loggeo de un segundo cliente
                // Ya que es un juego de 2 jugadores
                if (clients.size() > 0) {
                    ConectaCuatro_Message::MessageType messageType = ConectaCuatro_Message::CLIENT_LOGIN_REJECTED;
                    msg.type = messageType;
                    msg.message = MessageText(messageType);
                    socket.send(msg,*client_ptr);
                    continue;
                }
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void GameClient::login()
{
    std::string msg;

    ConectaCuatro_Message em(nick, msg);
    em.type = ConectaCuatro_Message::LOGIN;

    socket.send(em, socket);
}

void GameClient::logout()
{
    // Completar
}

void GameClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        // Enviar al servidor usando socket
    }
}

void GameClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
    }
}

