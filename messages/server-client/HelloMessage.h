//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_HELLOMESSAGE_H
#define BOMBERMANSERVER_HELLOMESSAGE_H

#include "messages/IMessage.h"
#include "types.h"
#include "common.h"

namespace bomberman {
    struct HelloMessage : IMessage {
        MapSettings mapSettings;

        explicit HelloMessage(socket_t &socket) : mapSettings(socket) {}

        void print() const {
            using namespace std;

            cerr << "Hello message:\n";
            cerr << "Server name: " << mapSettings.serverName << '\n';
            cerr << "Size x = " << mapSettings.sizeX << " size y = " << mapSettings.sizeY << '\n';
            cerr << "Game length: " << mapSettings.gameLength << '\n';
            cerr << "Explosion radius: " << mapSettings.explosionRadius << '\n';
            cerr << "Bomb timer: " << mapSettings.bombTimer << "\n\n";
        }
    };
}
#endif //BOMBERMANSERVER_HELLOMESSAGE_H
