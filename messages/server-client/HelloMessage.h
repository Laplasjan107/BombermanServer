//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_HELLOMESSAGE_H
#define BOMBERMANSERVER_HELLOMESSAGE_H

#include "messages/IMessage.h"
#include "types.h"
#include "common.h"

namespace bomberman {
    struct HelloMessage : IMessage {
        MapSettings mapSettings;

        explicit HelloMessage(socket_t &socket) : mapSettings(socket) {}

        void print() const {

        }
    };
}
#endif //BOMBERMANSERVER_HELLOMESSAGE_H
