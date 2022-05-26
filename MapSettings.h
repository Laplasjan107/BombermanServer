//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_MAPSETTINGS_H
#define BOMBERMANSERVER_MAPSETTINGS_H

#include "types.h"
#include "common.h"

namespace bomberman {
    struct MapSettings {
        string serverName;
        players_count_t playersCount{};
        board_size_t sizeX{};
        board_size_t sizeY{};
        game_length_t gameLength{};
        explosion_radius_t explosionRadius{};
        bomb_timer_t bombTimer{};

        MapSettings() = default;

        MapSettings(socket_t &socket) {
            serverName = read_string(socket);
            read_number_inplace(socket, playersCount);
            read_number_inplace(socket, sizeX);
            read_number_inplace(socket, sizeY);
            read_number_inplace(socket, gameLength);
            read_number_inplace(socket, explosionRadius);
            read_number_inplace(socket, bombTimer);
        }
    };
}

#endif //BOMBERMANSERVER_MAPSETTINGS_H
