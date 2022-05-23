//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_LOBBYMESSAGE_H
#define BOMBERMANSERVER_LOBBYMESSAGE_H

#include "messages/common_includes.h"
#include "UDPMessage.h"
#include "Player.h"
#include "MapSettings.h"
#include "DrawMessageType.h"

namespace bomberman {
    class LobbyMessage : public UDPMessage {
    public:
        LobbyMessage(MapSettings &map, players_t &players) {
            loadNumber(static_cast<uint8_t>(DrawMessageType::Lobby));
            loadString(map.serverName);
            loadNumber(map.playersCount);
            loadNumber(map.sizeX);
            loadNumber(map.sizeY);
            loadNumber(map.gameLength);
            loadNumber(map.explosionRadius);
            loadNumber(map.bombTimer);
            loadPlayersMap(players);
        }
    };
}

#endif //BOMBERMANSERVER_LOBBYMESSAGE_H
