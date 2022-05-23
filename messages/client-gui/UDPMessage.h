//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_UDPMESSAGE_H
#define BOMBERMANSERVER_UDPMESSAGE_H

#include <cstdlib>
#include <iostream>
#include <boost/endian/conversion.hpp>
#include "Player.h"
#include "Position.h"
#include "GameStatus.h"
#include "common.h"
#include "types.h"
#include "DrawMessageType.h"


namespace bomberman {
    using boost::asio::ip::udp;

    class UDPMessage {
        static const constexpr size_t udpDatagramSize = 65507;
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

        static void loadPlayersPositions(const std::unordered_map<player_id_t, Position> &map) {
            loadNumber((map_size_t) map.size());
            for (auto& player: map) {
                loadNumber((player_id_t) player.first);
                loadPosition(player.second);
            }
        }

        static void loadPositionsList(const std::vector<Position> &positions) {
            loadNumber((list_size_t) positions.size());
            for (auto &e : positions)
                loadPosition(e);
        }

        static void loadBomb(const Bomb &bomb) {
            loadPosition(bomb.bombPosition);
            loadNumber(bomb.timer);
        }

        static void loadBombsList(const std::vector<Bomb> &bombs) {
            loadNumber((list_size_t) bombs.size());
            for (auto &e : bombs)
                loadBomb(e);
        }

        static void loadPosition(const Position &position) {
            loadNumber(position.positionX);
            loadNumber(position.positionY);
        }

        static void loadScores(const std::unordered_map<player_id_t, score_t> &scores) {
            loadNumber((list_size_t) scores.size());
            for (auto &e: scores) {
                loadNumber(e.first);
                loadNumber(e.second);
            }
        }

    public:
        static void clearBuffer() {
            loaded = 0;
        }

        static void loadGameStatus(const GameStatus &gameStatus) {
            clearBuffer();
            loadNumber(static_cast<uint8_t>(DrawMessageType::Game));
            loadString(gameStatus.serverName);
            loadNumber(gameStatus.sizeX);
            loadNumber(gameStatus.sizeY);
            loadNumber(gameStatus.gameLength);
            loadNumber(gameStatus.turn);
            loadPlayersMap(gameStatus.players);
            loadPlayersPositions(gameStatus.positions);
            loadPositionsList(gameStatus.blocks);
            loadBombsList(gameStatus.bombs);
            loadPositionsList(gameStatus.explosions);
            loadScores(gameStatus.scores);
        }

        static void sendAndClear(udp::socket &socket, udp::endpoint& endpoint) {
            for (int i = 0; i < loaded; ++i) {
                std::cout << (int) bufferUDP[i] << ' ';
            }
            std::cout << std::endl;
            socket.send_to(boost::asio::buffer(bufferUDP, loaded), endpoint);
            clearBuffer();
        }

    };

    size_t UDPMessage::loaded = 0;
    uint8_t UDPMessage::bufferUDP[UDPMessage::udpDatagramSize];
}

#endif //BOMBERMANSERVER_UDPMESSAGE_H
