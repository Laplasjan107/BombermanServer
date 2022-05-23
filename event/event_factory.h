//
// Created by Piotr Kamiński on 23/05/2022.
//

#ifndef BOMBERMANSERVER_EVENT_FACTORY_H
#define BOMBERMANSERVER_EVENT_FACTORY_H

#incldue <memory>
#include "types.h"
#include "common.h"
#include "events.h"

namespace bomberman {
    std::unique_ptr<Event> deserialize_event(socket_t &socket) {
        EventType type;
        read_number_inplace(socket, *(uint8_t *)(&type));
        switch (type) {
            case EventType::BlockPlaced:
                return std::make_unique<BombPlaced>(socket);
            case EventType::BombPlaced:
                break;
            case EventType::BombExploded:
                break;
            case EventType::PlayerMoved:
                break;
        }
    }
}


#endif //BOMBERMANSERVER_EVENT_FACTORY_H
