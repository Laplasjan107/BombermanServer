//
// Created by Piotr Kamiński on 19/05/2022.
//

#ifndef BOMBERMANSERVER_CLIENTMESSAGETYPE_H
#define BOMBERMANSERVER_CLIENTMESSAGETYPE_H

namespace bomberman {
    enum class ClientMessageType {
        Join = 0,
        PlaceBomb = 1,
        PlaceBlock = 2,
        Move = 3,
    };
}

#endif //BOMBERMANSERVER_CLIENTMESSAGETYPE_H
