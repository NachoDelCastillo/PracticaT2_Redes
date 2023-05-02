#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// SERIALIZAR Y DESERIALIZAR LOS DATOS
void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    // Serializar los campos type, nick y message en el buffer _data
    char* data_ptr = _data;
    memcpy(data_ptr, &type, sizeof(uint8_t));
    data_ptr += sizeof(uint8_t);
    memcpy(data_ptr, nick.c_str(), MAX_NICK_SIZE * sizeof(char));
    data_ptr += MAX_NICK_SIZE * sizeof(char);
    memcpy(data_ptr, message.c_str(), MAX_MESSAGE_SIZE * sizeof(char));
}

int ChatMessage::from_bin(char* bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void*>(_data), bobj, MESSAGE_SIZE);

    char nick_buffer[MAX_NICK_SIZE];
    char message_buffer[MAX_MESSAGE_SIZE];

    // Reconstruir la clase usando el buffer _data
    char* data_ptr = _data;
    memcpy(&type, data_ptr, sizeof(uint8_t));
    data_ptr += sizeof(uint8_t);
    memcpy(&nick_buffer, data_ptr, MAX_NICK_SIZE * sizeof(char));
    data_ptr += MAX_NICK_SIZE * sizeof(char);
    memcpy(&message_buffer, data_ptr, MAX_MESSAGE_SIZE * sizeof(char));

    nick = nick_buffer;
    message = message_buffer;

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */

        Socket* client_socket_ptr;
        ChatMessage msg;
        // Recibir mensajes y en función del tipo de mensaje
        socket.recv(msg, client_socket_ptr);

        // - LOGIN: Añadir al vector clients
        if (msg.type == ChatMessage::MessageType::LOGIN)
        {
            std::cout << msg.nick << " LOGIN\n";
            clients.push_back(std::unique_ptr<Socket>(std::move(client_socket_ptr)));
        }

        // - LOGOUT: Eliminar del vector clients
        else if (msg.type == ChatMessage::MessageType::LOGOUT)
        {
            std::cout << msg.nick << " LOGOUT\n";
            auto it = clients.begin();
            while (it != clients.end() && !(*(*it).get() == *client_socket_ptr))
            {
                it++;
            }
            clients.erase(it);
        }

        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
        else if (msg.type == ChatMessage::MessageType::MESSAGE)
        {
            std::cout << msg.nick << " MESSAGE\n";
            for (int i = 0; i < clients.size(); ++i)
            {
                if (!(*(clients[i].get()) == *client_socket_ptr))
                {
                    socket.send(msg, (*clients[i].get()));
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
    std::string empty_message;

    ChatMessage login_message(nick, empty_message);
    login_message.type = ChatMessage::LOGIN;

    socket.send(login_message, socket);
}

void ChatClient::logout()
{
    std::string empty_message;

    ChatMessage logout_message(nick, empty_message);
    logout_message.type = ChatMessage::LOGOUT;

    socket.send(logout_message, socket);
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        std::string msg;
        std::getline(std::cin, msg);
        if(msg == "q"){
            logout();
            break;
        }
        ChatMessage chatMsg(nick, msg);
        chatMsg.type = ChatMessage::MessageType::MESSAGE;
        // Enviar al servidor usando socket
        socket.send(chatMsg, socket);
    }
}

void ChatClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        ChatMessage chatMsg;
        socket.recv(chatMsg);
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
        std::cout << chatMsg.nick << ": " << chatMsg.message << "\n";
    }
}

