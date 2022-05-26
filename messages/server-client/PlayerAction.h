//
// Created by Piotr Kamiński on 26/05/2022.
//

#ifndef ROBOTS_CLIENT_PLAYERACTION_H
#define ROBOTS_CLIENT_PLAYERACTION_H

#include <cstdlib>

namespace bomberman {
    enum class PlayerAction : uint8_t {
        Join = 0,
        PlaceBomb = 1,
        PlaceBlock = 2,
        Move = 3,
    };
}

#endif //ROBOTS_CLIENT_PLAYERACTION_H
