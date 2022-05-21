//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_HELLOMESSAGE_H
#define BOMBERMANSERVER_HELLOMESSAGE_H

#include <boost/endian/conversion.hpp>
#include "messages/IMessage.h"
#include "types.h"

namespace bomberman {
    class HelloMessage : IMessage {
        string_lenght_t stringLength{};
        string serverName;
        players_count_t playersCount{};
        board_size_t sizeX{};
        board_size_t sizeY{};
        game_length_t gameLength{};
        explosion_radius_t explosionRadius{};
        bomb_timer_t bombTimer{};

    public:
        HelloMessage(socket &socket) {
            using namespace boost::asio;

            read(socket, buffer(&stringLength, sizeof(string_lenght_t)));
            boost::endian::endian_reverse_inplace(stringLength);
            serverName = string(stringLength + 1, '\0');
            read(socket, buffer(serverName.data(), stringLength));

            read(socket, buffer(&playersCount, sizeof(players_count_t)));
            boost::endian::endian_reverse_inplace(playersCount);

            read(socket, buffer(&sizeX, sizeof(board_size_t)));
            boost::endian::endian_reverse_inplace(sizeX);

            read(socket, buffer(&sizeY, sizeof(board_size_t)));
            boost::endian::endian_reverse_inplace(sizeY);

            read(socket, buffer(&gameLength, sizeof(game_length_t)));
            boost::endian::endian_reverse_inplace(gameLength);

            read(socket, buffer(&explosionRadius, sizeof(explosion_radius_t)));
            boost::endian::endian_reverse_inplace(explosionRadius);

            read(socket, buffer(&bombTimer, sizeof(bomb_timer_t)));
            boost::endian::endian_reverse_inplace(bombTimer);
        }

        void print() {
            using namespace std;

            cout << "Hello message:\n";
            cout << "Server name length: " << stringLength << '\n';
            cout << "Server name: " << serverName << '\n';
            cout << "Size x = " << sizeX << " size y = " << sizeY << '\n';
            cout << "Game length: " << gameLength << '\n';
            cout << "Explosion radius: " << explosionRadius << '\n';
            cout << "Bomb timer: " << bombTimer << "\n\n";
        }
    };
}
#endif //BOMBERMANSERVER_HELLOMESSAGE_H
