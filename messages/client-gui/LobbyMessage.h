//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_LOBBYMESSAGE_H
#define BOMBERMANSERVER_LOBBYMESSAGE_H

#include "messages/common_includes.h"
#include "Player.h"

namespace bomberman {
    class LobbyMessage : IMessage {
    public:
        string serverName;
        players_count_t playerCount;
        board_size_t sizeX;
        board_size_t sizeY;
        game_length_t gameLength;
        explosion_radius_t explosionRadius;
        bomb_timer_t  bombTimer;
        players_t players;

        LobbyMessage(socket_t &socket) {

        }
    };
}

#endif //BOMBERMANSERVER_LOBBYMESSAGE_H
