#include <string>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <memory>

#include "Serializable.h"
#include "Socket.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/**
 *  Mensaje del protocolo de la aplicación de Chat
 *
 *  +-------------------+
 *  | Tipo: uint8_t     | 0 (login), 1 (mensaje), 2 (logout)
 *  +-------------------+
 *  | Nick: char[8]     | Nick incluido el char terminación de cadena '\0'
 *  +-------------------+
 *  |                   |
 *  | Mensaje: char[80] | Mensaje incluido el char terminación de cadena '\0'
 *  |                   |
 *  +-------------------+
 *
 */
class ConectaCuatro_Message: public Serializable
{
public:
    static const size_t MESSAGE_SIZE = sizeof(char) * 88 + sizeof(uint8_t);

    enum MessageType
    {
        // Cliente -> Servidor 
        CLIENT_LOGIN,
        CLIENT_LOGOUT, // Tambien se envia del servidor al cliente para informar por la pantalla del cliente
        // Servidor -> Cliente 
        CLIENT_LOGIN_REJECTED,
        HOST_LOGOUT,


        // Cliente -> Servidor 
        CLIENT_INPUT,
        // Servidor -> Cliente 
        INVALIDINPUT,

        // Servidor -> Cliente 
        HOST_WIN,
        CLIENT_WIN,
        DRAW,
        CURRENTTAB, // Se envia al cliente para que imprima el estado actual del tablero
        CLIENT_GIVETURN
    };

    ConectaCuatro_Message(){};

    ConectaCuatro_Message(const std::string& n, const std::string& m):nick(n),message(m){};

    void to_bin();

    int from_bin(char * bobj);

    uint8_t type;

    std::string nick;
    std::string message;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 *  Clase para el servidor de chat
 */
class GameServer
{
public:
    GameServer(const char * s, const char * p): socket(s, p), hostNick(n)
    {
        // Crear las casillas vacias y meterlas en el tablero dependiendo del numero de filas/columnas
        for (int i = 0; i < COLS; i++)
        {
            std::vector<int> fullColumn;
            for (int i = 0; i < ROWS; i++)
                fullColumn.push_back(0);

            tab.push_back(fullColumn);
        }

        socket.bind();
    };

    /**
     *  Thread principal del servidor recive mensajes en el socket y
     *  lo distribuye a los clientes. Mantiene actualizada la lista de clientes
     */
    void do_messages();

    void input_thread();

private:


        // Envia el mensaje al cliente de que el servidor se cierra y despues se desconecta
    void HostLogout();

        // Renderiza en la pantalla del usuario del servidor el tablero actual  y envia
    // un mensaje con el tablero actual al cliente para que lo renderize en su pantalla
    void UpdateTab(bool showTurn = true);

        // Crea y envia un mensaje al cliente informandole de que es su turno
    void GiveClientTurn();


    // Posiciona una ficha del jugador especificado en el tablero, actualizandolo internamente
    void PlaceChips(int col, std::string playerNick);

    // Renderiza en la pantalla del usuario del servidor el tablero actual  y envia
    // un mensaje con el tablero actual al cliente para que lo renderize en su pantalla
    void UpdateTab(bool showTurn = true);

    // Devuelve true si el jugador de index "playerIndex" tiene 4 fichas seguidas, ganando la partida
    bool CheckWin(int playerIndex);


    // Comprueba si ha ganado cualquiera de los dos jugadores o si se ha quedado empate
    // Si alguna de estas dos condiciones es cierta, crea y envia los mensajes necesarios 
    // Si ninguna de las dos condiciones anteriores es cierta, devuelve false
    bool CheckEndGame();


    // Proces el input registrado por el usuario del servidor o por el cliente
    void ProcessInput(std::string input, std::string playerNick);

    // Posiciona una ficha del jugador especificado en el tablero, actualizandolo internamente
    void PlaceChips(int col, std::string playerNick);


    // Devuelve true si el input introducido es una columna valida para posicionar la ficha
    bool ValidInput(std::string input, int& out);

    /**
     *  Lista de clientes conectados al servidor de Chat, representados por
     *  su socket
     */
    std::vector<std::unique_ptr<Socket>> clients;

    // VARIABLES

    // RELACIONADAS CON LA CONEXION SERVIDOR-CLIENTE

    // Cliente conectado al servidor
    std::vector<std::unique_ptr<Socket>> clients;
    // Socket utilizado
    Socket socket;

    // RELACIONADAS CON EL FLUJO DE JUEGO

    const int ROWS = 2; // 6 <- valores de tablero del juego original
    const int COLS = 3; // 7 <-

        // Tablero del juego, almacena la posicion de las fichas de cada jugador en cada turno
    std::vector<std::vector<int>> tab;

    // Indica si es el turno del usuario del servidor
    bool myTurn = true;
};

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

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 *  Clase para el cliente de chat
 */
class GameClient
{
public:
    /**
     * @param s dirección del servidor
     * @param p puerto del servidor
     * @param n nick del usuario
     */
    GameClient(const char * s, const char * p, const char * n):socket(s, p),
        nick(n){};

    /**
     *  Envía el mensaje de login al servidor
     */
    void login();

    /**
     *  Envía el mensaje de logout al servidor
     */
    void logout();

    /**
     *  Rutina principal para el Thread de E/S. Lee datos de STDIN (std::getline)
     *  y los envía por red vía el Socket.
     */
    void input_thread();

    /**
     *  Rutina del thread de Red. Recibe datos de la red y los "renderiza"
     *  en STDOUT
     */
    void net_thread();

private:

    /**
     * Socket para comunicar con el servidor
     */
    Socket socket;

    /**
     * Nick del usuario
     */
    std::string nick;
};

