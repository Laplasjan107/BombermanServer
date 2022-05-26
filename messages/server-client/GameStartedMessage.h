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

        explicit GameStartedMessage(socket_t &socket) {
            map_size_t mapLength;

            read_number_inplace(socket, mapLength);
            for (map_size_t i = 0; i < mapLength; ++i) {
                player_id_t id;

                read_number_inplace(socket, id);
                Player player{socket};
                activePlayers[id] = player;
            }
        }
    };
}

#endif //BOMBERMANSERVER_GAMESTARTEDMESSAGE_H
