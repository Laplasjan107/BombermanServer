//
// Created by Piotr Kamiński on 23/05/2022.
//

#ifndef BOMBERMANSERVER_BOMB_H
#define BOMBERMANSERVER_BOMB_H

#include "types.h"
#include "Position.h"
#include "common.h"

namespace bomberman {
    struct Bomb {
        Position bombPosition;
        bomb_timer_t timer{};

        Bomb() = default;

        Bomb(Position position, bomb_timer_t time) : bombPosition(position), timer(time) {}
    };
}

#endif //BOMBERMANSERVER_BOMB_H
