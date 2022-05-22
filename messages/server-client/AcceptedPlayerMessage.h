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
        Player player;

        explicit AcceptedPlayerMessage(socket_t &socket) {
            player_id_t playerId;
            string playerName;

            read_number_inplace(socket, playerId);
            playerName = read_string(socket);

            player = Player(playerId, std::move(playerName));
        }

        void print() const {
            using namespace std;

            cout << "Accepted player message:\n";
            cout << "player id: " << (int) player.playerId << endl;
            cout << "player name: " << player.playerName << endl;
        }
    };
}

#endif //BOMBERMANSERVER_ACCEPTEDPLAYERMESSAGE_H
