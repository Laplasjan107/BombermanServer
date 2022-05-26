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
    struct BombExplodedEvent : Event {
        bomb_id_t bombId{};
        std::vector<player_id_t> robotsDestroyed;
        std::vector<Position> blocksDestroyed;

        BombExplodedEvent(socket_t &socket) {
            read_number_inplace(socket, bombId);
            list_size_t destroyedPlayersCount;
            read_number_inplace(socket, destroyedPlayersCount);
            for (list_size_t i = 0; i < destroyedPlayersCount; ++i) {
                player_id_t destroyedPlayer;
                read_number_inplace(socket, destroyedPlayer);
                robotsDestroyed.push_back(destroyedPlayer);
            }

            list_size_t destroyedBlocksCount;
            read_number_inplace(socket, destroyedBlocksCount);
            for (list_size_t i = 0; i < destroyedBlocksCount; ++i) {
                Position destroyedBlock{socket};
                blocksDestroyed.push_back(destroyedBlock);
            }
        }
    };
}


#endif //BOMBERMANSERVER_BOMBEXPLODED_H
