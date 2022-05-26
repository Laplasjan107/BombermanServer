//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_GAMEENDEDMESSAGE_H
#define BOMBERMANSERVER_GAMEENDEDMESSAGE_H

#include "messages/IMessage.h"
#include "common.h"

namespace bomberman {
    struct GameEndedMessage : IMessage {
        GameEndedMessage(socket_t &socket) {
            map_size_t scoresSize;
            read_number_inplace(socket, scoresSize);
            for (map_size_t i = 0; i < scoresSize; ++i) {
                player_id_t playerId;
                score_t score;
                read_number_inplace(socket, playerId);
                read_number_inplace(socket, score);
            }
        }
    };
}

#endif //BOMBERMANSERVER_GAMEENDEDMESSAGE_H
