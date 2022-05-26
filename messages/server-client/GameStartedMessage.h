//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_GAMESTARTEDMESSAGE_H
#define BOMBERMANSERVER_GAMESTARTEDMESSAGE_H

#include "messages/common_includes.h"
#include "Player.h"

namespace bomberman {
    class GameStartedMessage : IMessage {
    public:
        players_t activePlayers;

        GameStartedMessage(socket_t &socket) {
            map_size_t mapLength;

            read_number_inplace(socket, mapLength);
            for (map_size_t i = 0; i < mapLength; ++i) {
                player_id_t id;

                read_number_inplace(socket, id);
                Player player {socket};
                activePlayers[id] = player;
            }
        }

        void print() const {
            using namespace std;

            cout << "Game started message:\n";
            cout << "Active players count: " << activePlayers.size() << endl;
            for (auto& player: activePlayers) {
                player.second.print();
                cout << endl;
            }
        }
    };
}

#endif //BOMBERMANSERVER_GAMESTARTEDMESSAGE_H
