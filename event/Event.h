//
// Created by Piotr Kamiński on 23/05/2022.
//

#ifndef BOMBERMANSERVER_EVENT_H
#define BOMBERMANSERVER_EVENT_H

#include "EventType.h"

namespace bomberman {
    struct Event {
        virtual ~Event() {}
    };
}

#endif //BOMBERMANSERVER_EVENT_H
