//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_BOMBEXPLODED_H
#define BOMBERMANSERVER_BOMBEXPLODED_H

#include "common.h"
#include "types.h"
#include <vector>
#include "Position.h"
#include "Event.h"

namespace bomberman {
    struct BombExploded : Event {
        bomb_id_t bombId{};
        std::vector<player_id_t> robotsDestroyed;
        std::vector<Position> blocksDestroyed;

        BombExploded(socket_t &socket) {
            read_number_inplace(socket, bombId);

            list_size_t robotsListSize;
            read_number_inplace(socket, robotsListSize);
            for (list_size_t i = 0; i < robotsListSize; ++i) {
                player_id_t destroyed;
                read_number_inplace(socket, destroyed);
                robotsDestroyed.push_back(destroyed);
            }

            list_size_t blocksListSize;
            for (list_size_t i = 0; i < blocksListSize; ++i) {
                blocksDestroyed.emplace_back(socket);
            }
        }
    };
}


#endif //BOMBERMANSERVER_BOMBEXPLODED_H
