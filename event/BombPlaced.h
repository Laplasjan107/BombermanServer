//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_BOMBPLACED_H
#define BOMBERMANSERVER_BOMBPLACED_H

#include "common.h"
#include "types.h"
#include "Position.h"
#include "Event.h"

namespace bomberman {
    struct BombPlacedEvent : Event {
        bomb_id_t bombId;
        Position position;

        BombPlacedEvent(socket_t &socket) {
            read_number_inplace(socket, bombId);
            position = Position {socket};
        }
    };
}

#endif //BOMBERMANSERVER_BOMBPLACED_H
