//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_SERVERMESSAGETYPE_H
#define BOMBERMANSERVER_SERVERMESSAGETYPE_H

#include "types.h"
#include <cstdlib>

namespace bomberman {
    enum class ServerMessageType : uint8_t {
        Hello = 0,
        AcceptedPlayer = 1,
        GameStarted = 2,
        Turn = 3,
        GameEnded = 4,
    };
}


#endif //BOMBERMANSERVER_SERVERMESSAGETYPE_H
