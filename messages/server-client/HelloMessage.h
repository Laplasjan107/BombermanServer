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

        explicit HelloMessage(socket_t &socket) : mapSettings(socket) { }

        void print() const {
            using namespace std;

            cout << "Hello message:\n";
            cout << "Server name: " << mapSettings.serverName << '\n';
            cout << "Size x = " << mapSettings.sizeX << " size y = " << mapSettings.sizeY << '\n';
            cout << "Game length: " << mapSettings.gameLength << '\n';
            cout << "Explosion radius: " << mapSettings.explosionRadius << '\n';
            cout << "Bomb timer: " << mapSettings.bombTimer << "\n\n";
        }
    };
}
#endif //BOMBERMANSERVER_HELLOMESSAGE_H
