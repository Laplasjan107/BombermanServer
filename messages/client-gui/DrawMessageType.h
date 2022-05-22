//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_DRAWMESSAGETYPE_H
#define BOMBERMANSERVER_DRAWMESSAGETYPE_H

#include <cstdlib>

namespace bomberman {
    enum class DrawMessageType : uint8_t {
        Lobby = 0,
        Game = 1,
    };
}

#endif //BOMBERMANSERVER_DRAWMESSAGETYPE_H
