//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_EVENTTYPE_H
#define BOMBERMANSERVER_EVENTTYPE_H

#include <cstdlib>
#include "types.h"

namespace bomberman {
    enum EventType : message_header_t {
        BombPlaced = 0,
        BombExploded = 1,
        PlayerMoved = 2,
        BlockPlaced = 3,
    };
}

#endif //BOMBERMANSERVER_EVENTTYPE_H
