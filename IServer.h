//
// Created by Piotr Kamiński on 03/06/2022.
//

#ifndef ROBOTS_CLIENT_ISERVER_H
#define ROBOTS_CLIENT_ISERVER_H

#include "types.h"

namespace bomberman {
    struct IServer {
        virtual void sendToAll(std::vector<uint8_t> message) = 0;

        virtual void sendTurn(std::vector<uint8_t> turnHeader, turn_message events) = 0;

        virtual ~IServer() {};
    };
}

#endif //ROBOTS_CLIENT_ISERVER_H
