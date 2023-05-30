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
        LOGIN   = 0,
        MESSAGE = 1,
        LOGOUT  = 2
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

