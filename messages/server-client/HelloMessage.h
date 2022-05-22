//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_HELLOMESSAGE_H
#define BOMBERMANSERVER_HELLOMESSAGE_H

#include "messages/IMessage.h"
#include "types.h"
#include "common.h"

namespace bomberman {
    class HelloMessage : IMessage {
        string serverName;
        players_count_t playersCount{};
        board_size_t sizeX{};
        board_size_t sizeY{};
        game_length_t gameLength{};
        explosion_radius_t explosionRadius{};
        bomb_timer_t bombTimer{};

    public:
        explicit HelloMessage(socket_t &socket) {
            serverName = read_string(socket);
            read_number_inplace(socket, playersCount);
            read_number_inplace(socket, sizeX);
            read_number_inplace(socket, sizeY);
            read_number_inplace(socket, gameLength);
            read_number_inplace(socket, explosionRadius);
            read_number_inplace(socket, bombTimer);
        }

        void print() {
            using namespace std;

            cout << "Hello message:\n";
            cout << "Server name: " << serverName << '\n';
            cout << "Size x = " << sizeX << " size y = " << sizeY << '\n';
            cout << "Game length: " << gameLength << '\n';
            cout << "Explosion radius: " << explosionRadius << '\n';
            cout << "Bomb timer: " << bombTimer << "\n\n";
        }
    };
}
#endif //BOMBERMANSERVER_HELLOMESSAGE_H
