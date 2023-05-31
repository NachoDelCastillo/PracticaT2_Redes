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
v
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


            // CLIENT_LOGIN
            if (msg.type == ConectaCuatro_Message::MessageType::CLIENT_LOGIN) {

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

            
                // En caso de que sea el primer cliente que se conecta, añadirlo a la lista de clientes y apuntar su nombre
                else {

                    // Añadir usuario a la lista de los clientes
                    clients.clear();
                    clients.push_back(std::move(client_ptr));
                    clientNick = msg.nick;

                    // Poner en pantalla el jugador que se ha unido
                    std::cout << std::endl << MessageText(ConectaCuatro_Message::MessageType::CLIENT_LOGIN) << std::endl << std::endl;

                    // Resetear variables del juego
                    myTurn = true;
                    for (size_t i = 0; i < tab.size(); i++)
                        for (size_t j = 0; j < tab[0].size(); j++)
                            tab[i][j] = 0;
                }

                        // CLIENT_LOGOUT
            else if (msg.type == ConectaCuatro_Message::MessageType::CLIENT_LOGOUT)
            {
                ConectaCuatro_Message::MessageType messageType = ConectaCuatro_Message::CLIENT_LOGOUT;
                msg.type = messageType;
                msg.message = MessageText(messageType);
                socket.send(msg,*clients[0]);
                std::cout << msg.message << std::endl;

                // El turno se queda en false para que no pueda ejecutar ninguna accion relacionada
                // con el transcurso de la partida abandonada
                myTurn = false;
                // Eliminar el cliente del servidor
                clients.clear();
            }

            else if (msg.type == ConectaCuatro_Message::MessageType::CLIENT_INPUT)
                // Es un input enviado desde el cliente
                ProcessInput(msg.message, msg.nick);
    }
}

void ConectaCuatro_Server::ProcessInput(std::string input, std::string playerNick) {

    // Comprobar si el input recibido es valido

    // En este entero se almacenara el resultado del numero entero en caso de
    // que sea valido
    int result;

    // Si es valido
    if (ValidInput(input, result)) {

        // Colocar la ficha
        PlaceChips(result, playerNick);

        // Ahora se pasara el turno al otro jugador
        // Lo que depende de que jugador a posicionado la ultima ficha en el tablero

        // Si el jugador que ha posicionado la ficha es el usuario del servidor
        if(playerNick == hostNick) {
            // Dejar de tener el turno
            myTurn = false;
            // Notificar al cliente de que es su turno
            GiveClientTurn();
        }

        // Si el jugador que ha posicionado la ficha es el cliente
        else 
            // Notificar al propio usuario del servidor de que es su turno
            myTurn = true;


        // Renderizar el tablero en las pantallas de ambos jugadores
        UpdateTab();
    }
}

void ConectaCuatro_Server::PlaceChips(int col, std::string playerNick) {

    // Saber a que index de jugador pertenece el nombre de jugador introducido

    // Aqui se almacenara el index del jugador al que le pertenece esta accion
    int playerIndex;
    
    // Si el jugador que ha realizado esta accion es el propio host, el index es 1
    if(playerNick == hostNick)
        playerIndex = 1;

    // Si el jugador que ha realizado esta accion es el cliente, el index es 2    
    else
        playerIndex = 2;

    // Posicionar la ficha en el primer hueco libre de la columna seleccionada
    for (int i = 0; i < ROWS; ++i) {
        if (tab[col][i] == 0) {
            tab[col][i] = playerIndex;
            break;
        }
    }

    // Si se ha posicionado una ficha, notificarlo al servidor
    chipsInTab++;
}


bool ConectaCuatro_Server::CheckWin(int playerIndex) {
    // Check horizontal
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j <= COLS - 4; ++j) {
            if (tab[i][j] == playerIndex && tab[i][j+1] == playerIndex && 
                tab[i][j+2] == playerIndex && tab[i][j+3] == playerIndex) {
                return true;
            }
        }
    }
    
    // Check vertical
    for (int i = 0; i <= ROWS - 4; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (tab[i][j] == playerIndex && tab[i+1][j] == playerIndex && 
                tab[i+2][j] == playerIndex && tab[i+3][j] == playerIndex) {
                return true;
            }
        }
    }
    
    // Check diagonal (positive slope)
    for (int i = 0; i <= ROWS - 4; ++i) {
        for (int j = 0; j <= COLS - 4; ++j) {
            if (tab[i][j] == playerIndex && tab[i+1][j+1] == playerIndex && 
                tab[i+2][j+2] == playerIndex && tab[i+3][j+3] == playerIndex) {
                return true;
            }
        }
    }
    
    // Check diagonal (negative slope)
    for (int i = 0; i <= ROWS - 4; ++i) {
        for (int j = COLS - 1; j >= 3; --j) {
            if (tab[i][j] == playerIndex && tab[i+1][j-1] == playerIndex && 
                tab[i+2][j-2] == playerIndex && tab[i+3][j-3] == playerIndex) {
                return true;
            }
        }
    }
    
    return false;
}


bool ConectaCuatro_Server::ValidInput(std::string input, int& result) {

    // Si el string no tiene longitud 1, no es un número entero válido
    if (input.length() != 1)
        return false; 

    // Ahora comprobar si ese unico char en el string constituye un numero entre 0 y 9
    char c = input[0];
    if (c >= '0' && c <= '9') {

        // Convertir el carácter a un número entero
        int num = c - '0';

        // Comprobar si ese numero es menor que el numero de columnas preestablecidas
        if (num < COLS) {

            // Por ultimo, falta comprobar si esa columna no esta al completo
            int chipsInThisColumn = 0;
            for (size_t i = 0; i < ROWS; i++)
                if (tab[num][i] != 0)
                    chipsInThisColumn++;
            
            // Si al menos cabe una ficha mas en esta columna, darlo como valido
            if (chipsInThisColumn < ROWS) {

                // Asignar la variable de salida y notificar que este input es valido
                result = num;
                return true;
            }
        }
    }

    return false;
}

void ConectaCuatro_Server::GiveClientTurn() {
    ConectaCuatro_Message msg;
    msg.type = ConectaCuatro_Message::MessageType::CLIENT_GIVETURN;
    socket.send(msg,*clients[0]);
}

void ConectaCuatro_Server::UpdateTab(bool showTurn) {

    // Envia el estado actual del tablero al cliente para que lo renderize en su pantalla
    ConectaCuatro_Message msg;
    msg.type = ConectaCuatro_Message::MessageType::CURRENTTAB;
    msg.message = CreateTab(showTurn);

    socket.send(msg,*clients[0]);

    // y lo renderiza en la pantalla del usuario del servidor
    std::cout << msg.message << std::endl;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void GameClient::login()
{
    std::string msg;
    ConectaCuatro_Message em(clientNick, msg);
    em.type = ConectaCuatro_Message::CLIENT_LOGIN;
    socket.send(em, socket);
}

void GameClient::logout()
{
    std::string msg;
    ConectaCuatro_Message em(clientNick,msg);
    em.type = ConectaCuatro_Message::CLIENT_LOGOUT;
    socket.send(em,socket);
}

void GameClient::input_thread()
{
    while (true)
    {
        // Recoger el input del cliente
        std::string clientInput;
        std::cin >> clientInput;
        
        if(clientInput == "q")
            logout();

        else {

            if (myTurn) {

                // El input generado por el cliente se manda al servidor, aunque sea invalido
                // (ya sea porque no es un numero de una columna existente, o la columna esta ya llena de fichas)
                // Se crea un mensaje con el nombre del cliente y el input generado
                ConectaCuatro_Message cmsg(clientNick,clientInput);

                // Se marca el tipo de mensaje y se envia
                cmsg.type= ConectaCuatro_Message::CLIENT_INPUT;
                socket.send(cmsg,socket);

                // Como el ciente ha enviado su input al servidor, tu turno a acabado, al no ser
                // que el servidor envie un mensaje de vuelta momentos despues de tipo INVALIDINPUT, 
                // y por lo tanto siga siendo el turno del cliente
                myTurn = false;
            }

        }
    }
}

void GameClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        ConectaCuatro_Message cmsg;
        socket.recv(cmsg);


        switch (cmsg.type)
        {
            // En caso de que el input enviado no haya sido valido, el servidor enviara este mensaje al cliente
            // para que vuelva a repetir su turno
            case ConectaCuatro_Message::MessageType::INVALIDINPUT:
                std::cout<<cmsg.message<<std::endl;
                myTurn = true;
                break;
                
            // Situacion del tablero actual, se envia desde el servidor cada vez que el cliente o el servidor ha modificado el tablero
            case ConectaCuatro_Message::MessageType::CURRENTTAB :
                std::cout<<cmsg.message<<std::endl;
                break;
            
            // Este mensaje se envia desde el servidor para saber cuando es el turno del cliente
            case ConectaCuatro_Message::MessageType::CLIENT_GIVETURN :
                myTurn = true;
                break;

            // Mensajes que marcan el final de la partida
            case ConectaCuatro_Message::MessageType::HOST_WIN : // Si el host ha ganado
            case ConectaCuatro_Message::MessageType::CLIENT_WIN : // Si el cliente (tu) has ganado
            case ConectaCuatro_Message::MessageType::DRAW : // Si el resultado es empate
                std::cout<<cmsg.message<<std::endl;
                // Si se ha terminado la partida de manera natural (sin que ningun usuario se salga de la partida)
                // Solicitar al servidor el logout inmediato
                logout();
                break;

                            // Mensaje no registrado en el cliente, imprimir su contenido de todas formas
            default:
                std::cout<<cmsg.message<<std::endl;
                break;
        }
        
    }
}

