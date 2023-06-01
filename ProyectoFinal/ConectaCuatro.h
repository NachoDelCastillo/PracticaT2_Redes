#include <string>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <memory>

#include "Serializable.h"
#include "Socket.h"

class ConectaCuatro_Message : public Serializable
{
public:
    static const size_t MESSAGE_SIZE = sizeof(char) * 308 + sizeof(uint8_t);

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
    ConectaCuatro_Message(const std::string &n, const std::string &m) : nick(n), message(m){};

    void to_bin();
    int from_bin(char *bobj);

    // El tipo del mensaje
    uint8_t type;

    // El nombre del usuario que ha enviado el mensaje
    std::string nick;

    // El contenido del mensaje
    std::string message;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 *  Clase para el servidor de chat
 */
class ConectaCuatro_Server
{ // Servidor-Cliente
public:
    // Servidor, Cliente, Nombre de usuario
    ConectaCuatro_Server(const char *s, const char *p, const char *n) : socket(s, p), hostNick(n)
    {
        // Crear las casillas vacias y meterlas en el tablero dependiendo del numero de filas/columnas
        for (int i = 0; i < COLS; i++)
        {
            std::vector<int> fullColumn;
            for (int i = 0; i < ROWS; i++)
                fullColumn.push_back(0);

            tab.push_back(fullColumn);
        }

        // Enlazar el socket
        socket.bind();
    };


    /**
     *  Thread principal del servidor recive mensajes en el socket y
     *  lo distribuye a los clientes. Mantiene actualizada la lista de clientes
     */
    void do_messages(); // Bucle principal del juego

    void input_thread();

private:

    // METODOS

    // Envia el mensaje al cliente de que el servidor se cierra y despues se desconecta
    void HostLogout();

    // Posiciona una ficha del jugador especificado en el tablero, actualizandolo internamente
    void PlaceChips(int col, std::string playerNick);

    // Renderiza en la pantalla del usuario del servidor el tablero actual  y envia
    // un mensaje con el tablero actual al cliente para que lo renderize en su pantalla
    void UpdateTab(bool showTurn = true);

    // Crea y envia un mensaje al cliente informandole de que es su turno
    void GiveClientTurn();

    // Crea la parte grafica del tablero en un string
    // Si la variable showTurn es true, tambien incluye de que jugador es el turno 
    std::string CreateTab(bool showTurn = true);

    // Devuelve true si el jugador de index "playerIndex" tiene 4 fichas seguidas, ganando la partida
    bool CheckWin(int playerIndex);

    // Proces el input registrado por el usuario del servidor o por el cliente
    void ProcessInput(std::string input, std::string playerNick);

    // Comprueba si ha ganado cualquiera de los dos jugadores o si se ha quedado empate
    // Si alguna de estas dos condiciones es cierta, crea y envia los mensajes necesarios 
    // Si ninguna de las dos condiciones anteriores es cierta, devuelve false
    bool CheckEndGame();

    // Devuelve true si el input introducido es una columna valida para posicionar la ficha
    bool ValidInput(std::string input, int& out);

    std::string MessageText(ConectaCuatro_Message::MessageType messageType);


    // VARIABLES

    // RELACIONADAS CON LA CONEXION SERVIDOR-CLIENTE

    // Cliente conectado al servidor
    std::vector<std::unique_ptr<Socket>> clients;
    // Socket utilizado
    Socket socket;

    // Almacena tanto el nombre de usuario del servidor como el del cliente
    std::string hostNick;
    std::string clientNick;


    // RELACIONADAS CON EL FLUJO DE JUEGO

    const int ROWS = 6; // 6 <- valores de tablero del juego original
    const int COLS = 7; // 7 <-

    // Tablero del juego, almacena la posicion de las fichas de cada jugador en cada turno
    std::vector<std::vector<int>> tab;

    // Indica si es el turno del usuario del servidor
    bool myTurn = true;

    // Lleva un recuento del numero de fichas que se han posicionado en el tablero
    // esto viene bien para comprobar cuando se ha rellenado el tablero de fichas
    int chipsInTab = 0;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 *  Clase para el cliente de chat
 */
class ConectaCuatro_Client
{
public:
    /**
     * @param s dirección del servidor
     * @param p puerto del servidor
     * @param n nick del usuario
     */
    ConectaCuatro_Client(const char *s, const char *p, const char *n) : socket(s, p), clientNick(n){};

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

    std::string clientNick;

    // Indica si es el turno del cliente
    bool myTurn = false;
};