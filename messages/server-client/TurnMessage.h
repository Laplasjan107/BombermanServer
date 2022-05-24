//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_TURNMESSAGE_H
#define BOMBERMANSERVER_TURNMESSAGE_H

#include "messages/IMessage.h"
#include "types.h"
#include "common.h"

namespace bomberman {
    struct TurnMessage : IMessage {
        game_length_t turn;
        std::vector<Event> events;

        explicit TurnMessage(socket_t &socket) {
            read_number_inplace(socket, turn);

        }
    };
}

#endif //BOMBERMANSERVER_TURNMESSAGE_H
