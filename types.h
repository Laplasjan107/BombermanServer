//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_TYPES_H
#define BOMBERMANSERVER_TYPES_H

#include <cstdlib>
#include <string>
#include <boost/asio.hpp>
#include <unordered_map>

namespace bomberman {
    using string_length_t = uint8_t;
    using players_count_t = uint8_t;
    using board_size_t = uint16_t;
    using game_length_t = uint16_t;
    using explosion_radius_t = uint16_t;
    using bomb_timer_t = uint16_t;
    using string = std::string;
    using socket_t = boost::asio::ip::tcp::socket;
    using player_id_t = uint8_t;
    using map_size_t = uint32_t;
}

#endif //BOMBERMANSERVER_TYPES_H
