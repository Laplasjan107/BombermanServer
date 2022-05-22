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
        string playerName;
        string playerAddress;

        Player() = default;

        Player(socket_t &socket) {
            playerName = read_string(socket);
            playerAddress = read_string(socket);
        }

        Player(string name, string address) : playerName(std::move(name)), playerAddress(std::move(address)) { }

        void print() const {
            using namespace std;

            cout << "PLAYER: name = " << playerName << " address = " << playerAddress << endl;
        }
    };

    using players_t = std::unordered_map<player_id_t, Player>;
}

#endif //BOMBERMANSERVER_PLAYER_H
