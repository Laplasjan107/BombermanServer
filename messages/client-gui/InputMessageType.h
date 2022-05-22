//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_INPUTMESSAGETYPE_H
#define BOMBERMANSERVER_INPUTMESSAGETYPE_H

#include <cstdlib>

namespace bomberman {
    enum class InputMessageType : uint8_t {
        PlaceBomb = 0,
        PlaceBlock = 1,
        Move = 2,
    };
}

#endif //BOMBERMANSERVER_INPUTMESSAGETYPE_H
