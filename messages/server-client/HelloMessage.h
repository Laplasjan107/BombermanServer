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
        MapSettings map;

        explicit HelloMessage(socket_t &socket) : map(socket) { }

        void print() const {
            using namespace std;

            cout << "Hello message:\n";
            cout << "Server name: " << map.serverName << '\n';
            cout << "Size x = " << map.sizeX << " size y = " << map.sizeY << '\n';
            cout << "Game length: " << map.gameLength << '\n';
            cout << "Explosion radius: " << map.explosionRadius << '\n';
            cout << "Bomb timer: " << map.bombTimer << "\n\n";
        }
    };
}
#endif //BOMBERMANSERVER_HELLOMESSAGE_H
