//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_ACCEPTEDPLAYERMESSAGE_H
#define BOMBERMANSERVER_ACCEPTEDPLAYERMESSAGE_H

#include <iostream>
#include "messages/IMessage.h"
#include "common.h"
#include "types.h"

namespace bomberman {
    class AcceptedPlayerMessage : IMessage {
        string playerId;
        string playerName;

    public:
        explicit AcceptedPlayerMessage(socket_t &socket) {
            playerId = read_string(socket);
            playerName = read_string(socket);
        }

        void print() {
            using namespace std;
            cout << "player id = " << playerId << endl;
            cout << "player name = " << playerName << endl;
        }
    };
}

#endif //BOMBERMANSERVER_ACCEPTEDPLAYERMESSAGE_H
