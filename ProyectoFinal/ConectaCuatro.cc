#include "ConectaCuatro.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ConectaCuatro_Message::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    // Serializar los campos type, nick y message en el buffer _data
    char *tmp = _data;

    // Identificar el tipo de mensaje 
    memcpy(tmp, &type, sizeof(uint8_t));
    tmp +=sizeof(uint8_t);

    // Espacio para el nombre
    memcpy(tmp, nick.c_str(), sizeof(char) * 8);
    tmp += 8*sizeof(char);

    // Importante que el tamaño del mensaje pueda almacenar 
    // el estado del tablero entero, invluyendo los "|" 
    memcpy(tmp, message.c_str(), sizeof(char) * 300);
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

void ConectaCuatro_Server::do_messages()
{        
    while (true)
    {
            // Recibir mensajes del cliente conectado
            ConectaCuatro_Message msg;
            Socket *client = new Socket(socket);
            socket.recv(msg,client);

            // POSIBLES MENSAJES QUE PUEDE RECIBIR EL SERVIDOR DESDE EL CLIENTE
            // CLIENT_LOGIN
            // CLIENT_LOGOUT
            // CLIENT_INPUT

            // CLIENT_LOGIN
            if (msg.type == ConectaCuatro_Message::MessageType::CLIENT_LOGIN) {

                std::unique_ptr<Socket> client_ptr = std::make_unique<Socket>(*client);

                // Si ya se ha registrado un cliente, no permitir el Loggeo de un segundo cliente
                // Ya que es un juego de 2 jugadores
                if (clients.size() > 0) {
                    ConectaCuatro_Message::MessageType messageType = ConectaCuatro_Message::CLIENT_LOGIN_REJECTED;
                    msg.type = messageType;
                    msg.message = MessageText(messageType);
                    socket.send(msg,*client_ptr);
                    continue;
                }

                // En caso de que sea el primer cliente que se conecta, añadirlo a la lista de clientes y apuntar su nombre
                else {

                    // Añadir usuario a la lista de los clientes
                    clients.clear();
                    clients.push_back(std::move(client_ptr));
                    clientNick = msg.nick;

                    // En el caso en el que los dos usuarios hayan puesto el mismo nombre, el nick del usuario del servidor
                    // especificara que el es el host cuando se escriba su nombre en pantalla
                    if (hostNick == clientNick)
                        hostNick = "(host) " + hostNick;

                    // Poner en pantalla el jugador que se ha unido
                    std::cout << std::endl << MessageText(ConectaCuatro_Message::MessageType::CLIENT_LOGIN) << std::endl << std::endl;

                    // Resetear variables del juego
                    myTurn = true;
                    for (size_t i = 0; i < tab.size(); i++)
                        for (size_t j = 0; j < tab[0].size(); j++)
                            tab[i][j] = 0;
                }

                // Renderizar el tablero vacio en las pantallas de ambos jugadores
                UpdateTab();
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

            // CLIENT_INPUT
            else if (msg.type == ConectaCuatro_Message::MessageType::CLIENT_INPUT)
                // Es un input enviado desde el cliente
                ProcessInput(msg.message, msg.nick);
    }
}

void ConectaCuatro_Server::input_thread(){

    while (true)
    {
        // Recoger el input del usuario del servidor
        std::string hostInput;
        std::cin >> hostInput;

        // Comprobar si el input del host es para un LOGOUT
        if(hostInput == "q")
            HostLogout();

        // Si no es el input de LOGOUT, tiene que ser input relacionado con la partida
        else {

            // Solo se tendra en cuenta el input sobre la partida si es su turno
            if (myTurn)
                ProcessInput(hostInput, hostNick);
        }
    }
}

void ConectaCuatro_Server::HostLogout() {

    ConectaCuatro_Message msg;
    ConectaCuatro_Message::MessageType messageType = ConectaCuatro_Message::MessageType::HOST_LOGOUT;
    msg.type = messageType;
    msg.message = MessageText(messageType);
    // Comprobar si hay algun cliente conectado, en caso de que lo haya, 
    // informarle de que el usuario del servidor se ha desconectado
    if (clients.size() > 0)
        socket.send(msg,*clients[0]);
    std::cout << msg.message << std::endl;

    // Terminar ejecucion en el servidor
    exit(0);
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

        // Antes de decidir de quien es el turno ahora, comprobar si alguno de los dos jugadores ha ganado
        if (CheckEndGame()) {
            // Poner el turno a false para inhabilitar el input en el jugador mientras se busca una nueva partida
            myTurn = false;
            return;
        }

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

    // Si el input introducido no es valido
    else {
        
        // Se tiene que comprobar que jugador ha realizado el error, si el servidor o el cliente

        // Si el jugador que ha realizado el error es el propio host
        if(playerNick == hostNick)
            std::cout << MessageText(ConectaCuatro_Message::MessageType::INVALIDINPUT) << std::endl;

        // Si el jugador que ha realizado el error es el cliente  
        else {
            // Notificarle de que el input que envio al servidor, es erroneo y se espera otro input
            ConectaCuatro_Message msg;
            ConectaCuatro_Message::MessageType messageType = ConectaCuatro_Message::MessageType::INVALIDINPUT;
            msg.type = messageType;
            msg.message = MessageText(messageType);
            socket.send(msg,*clients[0]);
        }
    }
}

bool ConectaCuatro_Server::CheckEndGame() {

    // Comprobar si ha ganado el usuario del servidor
    if (CheckWin(1)) {

        // Renderizar el estado del tablero terminado, sin decir de quien era el turno
        UpdateTab(false);

        // Crear y enviar un mensaje informando al cliente
        ConectaCuatro_Message msg;
        ConectaCuatro_Message::MessageType messageType = ConectaCuatro_Message::MessageType::HOST_WIN;
        msg.type = messageType;
        msg.message = MessageText(messageType);
        socket.send(msg,*clients[0]);

        // Imprimir en la pantalla del usuario del servidor el mensaje
        std::cout << msg.message << std::endl;

        return true;
    }

    // Comprobar si ha ganado el cliente
    else if (CheckWin(2)) {

        // Renderizar el estado del tablero terminado, sin decir de quien era el turno
        UpdateTab(false);

        // Crear y enviar un mensaje informando al cliente
        ConectaCuatro_Message msg;
        ConectaCuatro_Message::MessageType messageType = ConectaCuatro_Message::MessageType::CLIENT_WIN;
        msg.type = messageType;
        msg.message = MessageText(messageType);
        socket.send(msg,*clients[0]);

        // Imprimir en la pantalla del usuario del servidor el mensaje
        std::cout << msg.message << std::endl;

        return true;
    }

    // Comprobar si es empate (si no queda mas espacio en el tablero)
    else if (chipsInTab == COLS * ROWS) {

        // Renderizar el estado del tablero terminado, sin decir de quien era el turno
        UpdateTab(false);

        // Crear y enviar un mensaje informando al cliente
        ConectaCuatro_Message msg;
        ConectaCuatro_Message::MessageType messageType = ConectaCuatro_Message::MessageType::DRAW;
        msg.type = messageType;
        msg.message = MessageText(messageType);
        socket.send(msg,*clients[0]);

        // Imprimir en la pantalla del usuario del servidor el mensaje
        std::cout << msg.message << std::endl;

        return true;
    }

    // Si el juego todavia no se ha acabado, devolver false
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

std::string ConectaCuatro_Server::MessageText(ConectaCuatro_Message::MessageType messageType) {

    switch (messageType)
    {
        case ConectaCuatro_Message::MessageType::CLIENT_LOGIN:
            return "Player " + clientNick + " (client) has joined the game";
            break;

        case ConectaCuatro_Message::MessageType::CLIENT_LOGOUT:
            return "Player " + clientNick + " (client) has left the game";
            break;

        case ConectaCuatro_Message::MessageType::CLIENT_LOGIN_REJECTED:
            return "Cannot connect to server, the lobby is already full [2/2]";
            break;

        case ConectaCuatro_Message::MessageType::HOST_LOGOUT:
            return "The Host ended the game";
            break;

        case ConectaCuatro_Message::MessageType::INVALIDINPUT:
            return "Invalid Input : Only the numbers that can be seen under the columns are accepted \nAlso, be sure not to overload one column with chips";
            break;

        case ConectaCuatro_Message::MessageType::HOST_WIN:
            return hostNick + " (host) won the game";
            break;

        case ConectaCuatro_Message::MessageType::CLIENT_WIN:
            return clientNick + "(client) won the game";
            break;

        case ConectaCuatro_Message::MessageType::DRAW:
            return "There was a draw";
            break;

        default:
            return "";
        break;
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

void ConectaCuatro_Server::UpdateTab(bool showTurn) {

    // Envia el estado actual del tablero al cliente para que lo renderize en su pantalla
    ConectaCuatro_Message msg;
    msg.type = ConectaCuatro_Message::MessageType::CURRENTTAB;
    msg.message = CreateTab(showTurn);

    socket.send(msg,*clients[0]);

    // y lo renderiza en la pantalla del usuario del servidor
    std::cout << msg.message << std::endl;
}

void ConectaCuatro_Server::GiveClientTurn() {
    ConectaCuatro_Message msg;
    msg.type = ConectaCuatro_Message::MessageType::CLIENT_GIVETURN;
    socket.send(msg,*clients[0]);
}

std::string ConectaCuatro_Server::CreateTab(bool showTurn){

    std::string tablero = "";

    std::string offset = "     ";
    // Marco de arriba
    tablero += "-_-_-_-_-_-_-_-_-_-_-_-_- \n \n";

    // Poner de quien es el turno si se pide
    if (showTurn) {
        std::string userTurn;
        if(myTurn)
            userTurn = hostNick;
        else 
            userTurn = clientNick;
        tablero += (offset + userTurn + "'s turn \n");
    }

    // Añadir borde superior del tablero
    tablero += offset;
    for(int i=0; i<COLS;i++) {
        tablero.push_back(' ');
        tablero.push_back('_');
    }
    tablero.push_back('\n');

    // Se renderiza de abajo a arriba, para que la gravedad sea hacia abajo
    // en vez de hacia arriba
    for(int i = ROWS-1; i >= 0 ;i--){

        tablero += offset;
        // Borde izquierdo del tablero
        tablero.push_back('|');
        // Espacios en esas filas
        // Numero de columnas, y por lo tanto posibles opciones
        for(int j=0; j<COLS;j++){

            if(tab[j][i]==1) tablero.push_back('X');
            else if(tab[j][i]==2) tablero.push_back('O');
            else tablero.push_back('_');

            // Lineas verticales intermedias
            tablero.push_back('|');
        }
        tablero.push_back('\n');
    }

    // Indicar el numero de cada columna para clarificar las posibles opciones
    tablero += offset;
    for(int i=0; i < COLS; i++) {
        tablero.push_back(' ');
        tablero.push_back(std::to_string(i)[0]);
    }
    tablero.push_back('\n');

    // Marco de abajo
    tablero += "\n -_-_-_-_-_-_-_-_-_-_-_-_- \n";

    return tablero;
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

void ConectaCuatro_Client::login()
{
    std::string msg;
    ConectaCuatro_Message em(clientNick, msg);
    em.type = ConectaCuatro_Message::CLIENT_LOGIN;
    socket.send(em, socket);
}

void ConectaCuatro_Client::logout()
{
    std::string msg;
    ConectaCuatro_Message em(clientNick,msg);
    em.type = ConectaCuatro_Message::CLIENT_LOGOUT;
    socket.send(em,socket);

    // No se desconecta directamente ya que se esperara a que el servidor confirme la desconexion.
}

void ConectaCuatro_Client::input_thread()
{
    while (true)
    {
        // Recoger el input del cliente
        std::string clientInput;
        std::cin >> clientInput;

        // Comprobar si el input introducido es de para cerrar conexion
        if(clientInput == "q")
            logout();
        
        else {

            // Si el input del cliente no es de tipo LOGOUT, solo puede ser relevante para la partida
            // Por lo que solo se tendra en cuenta el mensaje si es el turno del cliente
            // si no, ni siquiera se notificara al servidor del mensaje
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

void ConectaCuatro_Client::net_thread()
{
    // Mientras el cliente se mantenga en la conexion
    while(true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
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

            case ConectaCuatro_Message::MessageType::CLIENT_LOGIN_REJECTED: // Si se ha intentado conectar al servidor, pero estaba lleno
            case ConectaCuatro_Message::MessageType::HOST_LOGOUT: // Si el host se ha desconectado
            case ConectaCuatro_Message::MessageType::CLIENT_LOGOUT: // Si el host ha procesado correctamente tu deslogeo
                std::cout<<cmsg.message<<std::endl;
                exit(0);
                break;

            // Mensaje no registrado en el cliente, imprimir su contenido de todas formas
            default:
                std::cout<<cmsg.message<<std::endl;
                break;
        }
    }
}