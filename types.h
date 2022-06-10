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
    using string = std::string;

    using message_header_t = uint8_t;
    using string_length_t = uint8_t;
    using players_count_t = uint8_t;
    using board_size_t = uint16_t;
    using game_length_t = uint16_t;
    using explosion_radius_t = uint16_t;
    using bomb_timer_t = uint16_t;
    using socket_t = boost::asio::ip::tcp::socket;
    using player_id_t = uint8_t;
    using map_size_t = uint32_t;
    using list_size_t = map_size_t;
    using bomb_id_t = uint32_t;
    using score_t = uint32_t;

    using event_message = std::vector<uint8_t>;
    using turn_message = std::vector<event_message>;
    const constexpr size_t numberOfDirections = 4;
}

#endif //BOMBERMANSERVER_TYPES_H
