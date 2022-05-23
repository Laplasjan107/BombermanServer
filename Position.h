//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_POSITION_H
#define BOMBERMANSERVER_POSITION_H

#include "types.h"
#include "common.h"

namespace bomberman {
    struct Position {
        board_size_t positionX{};
        board_size_t positionY{};

        Position() = default;

        Position(socket_t &socket) {
            read_number_inplace(socket, positionX);
            read_number_inplace(socket, positionY);
        }
    };
}

#endif //BOMBERMANSERVER_POSITION_H
