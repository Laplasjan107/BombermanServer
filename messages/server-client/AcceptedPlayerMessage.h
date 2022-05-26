//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_ACCEPTEDPLAYERMESSAGE_H
#define BOMBERMANSERVER_ACCEPTEDPLAYERMESSAGE_H

#include <iostream>
#include <utility>
#include "messages/IMessage.h"
#include "common.h"
#include "types.h"
#include "Player.h"

namespace bomberman {
    class AcceptedPlayerMessage : IMessage {
    public:
        player_id_t playerId;
        Player player;

        explicit AcceptedPlayerMessage(socket_t &socket) {
            read_number_inplace(socket, playerId);
            player = Player(socket);
        }

        void print() const {

        }
    };
}

#endif //BOMBERMANSERVER_ACCEPTEDPLAYERMESSAGE_H
