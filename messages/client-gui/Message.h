//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_UDPMESSAGE_H
#define BOMBERMANSERVER_UDPMESSAGE_H

#include <cstdlib>
#include <iostream>
#include <boost/endian/conversion.hpp>
#include <unordered_set>
#include "Player.h"
#include "Position.h"
#include "common.h"
#include "types.h"
#include "DrawMessageType.h"
#include "Bomb.h"


namespace bomberman {
    using boost::asio::ip::udp;

    template <typename T, typename U>
    concept loadable = requires(T t, U u) {
        t << u;
    };

    class Message {
        static const constexpr size_t udpDatagramSize = 65507;
    public:
        static uint8_t bufferUDP[udpDatagramSize];
        static size_t loaded;

        template<typename T>
        static void loadNumber(T number) {
            if (loaded + sizeof(T) > udpDatagramSize)
                throw;

            T* toData = (T *) (bufferUDP + loaded);
            boost::endian::endian_reverse_inplace(number);
            *toData = number;
            loaded += sizeof(T);
        }

        static void loadString(const string &name) {
            if (loaded + name.length() + 1 > udpDatagramSize)
                throw;

            loadNumber((string_length_t) name.length());
            memcpy(bufferUDP + loaded, name.data(), name.length());
            loaded += name.length();
        }

        static void loadPlayer(const Player &player) {
            loadString(player.playerName);
            loadString(player.playerAddress);
        }

        static void loadPlayersMap(const players_t &map) {
            loadNumber((map_size_t) map.size());

            for (auto& player: map) {
                loadNumber((player_id_t) player.first);
                loadPlayer(player.second);
            }
        }

        static void loadPosition(const Position &position) {
            loadNumber(position.positionX);
            loadNumber(position.positionY);
        }


    public:
        static Message& getInstance() {
            static Message instance;
            return instance;
        }

       template<typename T>
       requires std::is_convertible_v<T, int>
       friend Message& operator<<(Message &message, T number) {
           if (loaded + sizeof(T) > udpDatagramSize)
               throw;

           T* toData = (T *) (bufferUDP + loaded);
           boost::endian::endian_reverse_inplace(number);
           *toData = number;
           loaded += sizeof(T);

           return message;
       }

       friend Message& operator<<(Message &message, const std::string &string) {
           if (loaded + string.length() + 1 > udpDatagramSize)
               throw;

           message << (string_length_t) string.length();
           memcpy(bufferUDP + loaded, string.data(), string.length());
           loaded += string.length();
           return message;
       }

        friend Message& operator<<(Message &message, const Player &player) {
            message << player.playerName << player.playerAddress;
            return message;
        }

        friend Message& operator<<(Message &message, const Position &position) {
            message << position.positionX << position.positionY;
            return message;
        }

        friend Message& operator<<(Message &message, const Bomb &bomb) {
            message << bomb.bombPosition << bomb.timer;
            return message;
        }

        template <typename T>
        requires loadable<Message, T>
        friend Message& operator<<(Message &message, const std::unordered_set<T> &elements) {
            message << (list_size_t) elements.size();
            for (auto &element: elements)
                message << element;
            return message;
        }

        template <typename T, typename U>
        requires loadable<Message, T> && loadable<Message, U>
        friend Message& operator<<(Message &message, const std::unordered_map<T, U> &map) {
            message << (map_size_t) map.size();
            for (auto &element: map)
                message << element.first << element.second;
            return message;
        }

        friend Message& operator<<(Message &message, const std::unordered_map<bomb_id_t, Bomb> &bombs) {
            message << (list_size_t) bombs.size();
            for (auto &element: bombs)
                message << element.second;
            return message;
        }

        static void clearBuffer() {
            loaded = 0;
        }

        static void sendAndClear(udp::socket &socket, udp::endpoint& endpoint) {
            socket.send_to(boost::asio::buffer(bufferUDP, loaded), endpoint);
            clearBuffer();
        }

        static auto getBuffer() {
            std::vector<uint8_t> send {std::begin(bufferUDP), std::begin(bufferUDP) + loaded};
            clearBuffer();
            return send;
        }
    };

    size_t Message::loaded = 0;
    uint8_t Message::bufferUDP[Message::udpDatagramSize];
}

#endif //BOMBERMANSERVER_UDPMESSAGE_H
