#include "Serializable.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

class Jugador: public Serializable
{
public:
    Jugador(const char * _n, int16_t _x, int16_t _y):x(_x),y(_y)
    {
        strncpy(name, _n, MAX_NAME);
        int32_t nameSize = MAX_NAME * sizeof(char);
        int32_t dataSize = nameSize + 2 * sizeof(int16_t);
        alloc_data(dataSize);
    };

    virtual ~Jugador(){};

    void to_bin()
    {
        int32_t nameSize = MAX_NAME * sizeof(char);
        int32_t dataSize = nameSize + 2 * sizeof(int16_t);

        char* aux = _data;
        memcpy(aux, name, nameSize);
        aux += nameSize;
        memcpy(aux, &x, sizeof(int16_t));
        aux += sizeof(int16_t);
        memcpy(aux, &y, sizeof(int16_t));
    }

    int from_bin(char * data)
    {
        int32_t nameSize = MAX_NAME * sizeof(char);

        char* aux = data;
        memcpy(name, aux, nameSize);
        aux += nameSize;
        memcpy(&x, aux, sizeof(int16_t));
        aux += sizeof(int16_t);
        memcpy(&y, aux, sizeof(int16_t));
        return 0;
    }

    public:
    static const size_t MAX_NAME = 20;
    char name[MAX_NAME];

    int16_t x;
    int16_t y;
};

int main(int argc, char **argv)
{
    Jugador one_r("", 0, 0);
    Jugador one_w("Player_ONE", 123, 987);

    // 1. Serializar el objeto one_w
    int fd = open("./player_data", O_CREAT | O_TRUNC | O_RDWR, 0666);
    one_w.to_bin();
    // 2. Escribir la serialización en un fichero
    write(fd, one_w.data(), one_w.size());

    // 3. Leer el fichero
    close(fd);
    fd = open("./player_data", O_RDONLY);

    char data[one_w.size()];

    read(fd, data, one_r.size());
    close(fd);


    // 4. "Deserializar" en one_r
    one_r.from_bin(data);
     // 5. Mostrar el contenido de one_r
    std::cout << "Nombre: " << one_r.name << " Posicion: {" << one_r.x << ", " << one_r.y << "}\n";

    return 0;
}
