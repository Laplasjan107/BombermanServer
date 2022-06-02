//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_PLAYER_H
#define BOMBERMANSERVER_PLAYER_H

#include <iostream>
#include <utility>
#include "types.h"
#include "common.h"
#include "Position.h"

namespace bomberman {
    struct Player {
        string playerName;
        string playerAddress;
        string buffer;

        Player() = default;

        explicit Player(socket_t &socket) {
            playerName = read_string(socket);
            playerAddress = read_string(socket);
        }

        Player(string name, string address) : playerName(std::move(name)), playerAddress(std::move(address)) {}

        const string &toBuffer() {
            if (buffer.empty()) {
                buffer.append(1, (char) playerName.size());
                buffer.append(playerName);
                buffer.append(1, (char) playerAddress.size());
                buffer.append(playerAddress);
            }

            return buffer;
        }

        void print() const {
        }
    };

    using players_t = std::unordered_map<player_id_t, Player>;
    using players_position_t = std::unordered_map<player_id_t, Position>;
}

#endif //BOMBERMANSERVER_PLAYER_H
