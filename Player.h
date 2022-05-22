//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_PLAYER_H
#define BOMBERMANSERVER_PLAYER_H

#include <iostream>
#include <utility>
#include "types.h"
#include "common.h"

namespace bomberman {
    struct Player {
        player_id_t playerId;
        string playerName;

        Player() = default;

        Player(socket_t &socket) {
            read_number_inplace(socket, playerId);
            playerName = read_string(socket);
        }

        Player(player_id_t id, string name) : playerId(id), playerName(std::move(name)) { }

        void print() const {
            using namespace std;

            cout << "PLAYER: id = " << playerId << " name = " << playerName << endl;
        }
    };

    using players_t = std::unordered_map<player_id_t, Player>;
}

#endif //BOMBERMANSERVER_PLAYER_H
