//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_HELLOMESSAGE_H
#define BOMBERMANSERVER_HELLOMESSAGE_H

#include <boost/asio.hpp>
#include <string>
#include "../IMessage.h"

class HelloMessage : IMessage {
    using string_lenght_t = uint16_t;
    using players_count_t = uint8_t;
    using board_size_t = uint16_t;
    using game_length_t = uint16_t;
    using explosion_radius_t = uint16_t;
    using bomb_timer_t = uint16_t;
    using string = std::string;
    using socket = boost::asio::ip::tcp::socket;

    string_lenght_t stringLength;
    string serverName;
    players_count_t playersCount;
    board_size_t sizeX;
    board_size_t sizeY;
    game_length_t gameLength;
    explosion_radius_t explosionRadius;
    bomb_timer_t bombTimer;

public:
    HelloMessage(socket &socket) {
        using namespace boost::asio;

        read(socket, buffer(&stringLength, sizeof(string_lenght_t)));
        serverName = string(nullptr, stringLength + 1);
        read(socket, buffer(serverName.data(), stringLength));
        read(socket, buffer(&playersCount, sizeof(players_count_t)));
        read(socket, buffer(&sizeX, sizeof(board_size_t)));
        read(socket, buffer(&sizeY, sizeof(board_size_t)));
        read(socket, buffer(&gameLength, sizeof(game_length_t)));
        read(socket, buffer(&explosionRadius, sizeof(explosion_radius_t)));
        read(socket, buffer(&bombTimer, sizeof(bomb_timer_t)));
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

#endif //BOMBERMANSERVER_HELLOMESSAGE_H
