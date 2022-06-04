//
// Created by Piotr Kamiński on 03/06/2022.
//

#ifndef ROBOTS_CLIENT_GAME_H
#define ROBOTS_CLIENT_GAME_H

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <unordered_set>
#include <set>
#include <map>
#include <queue>

#include "types.h"
#include "Player.h"
#include "messages/client-gui/UDPMessage.h"
#include "ServerOptions.h"
#include "Bomb.h"

namespace bomberman {
    struct Game {
        //event_message _events;
        turn_message _events;
        turn_message _allTurns;

        static size_t random() {
            static size_t last = 324;
            last *= 48271;
            last %= 2147483647;
            return last;
        }

        Position randomPosition() const {
            return {static_cast<board_size_t>(random() % _gameOptions.sizeX),
                    static_cast<board_size_t>(random() % _gameOptions.sizeY)};
        }

        std::vector<uint8_t> turns;
        static const constexpr Position versors[4] = {{0,                  1},
                                                      {1,                  0},
                                                      {0,                  (board_size_t) - 1},
                                                      {(board_size_t) - 1, 0},
        };


        explicit Game(GameOptions &gameOptions, std::shared_ptr<IServer> server) :
                _gameOptions(gameOptions),
                _isRunning(false),
                _server(std::move(server)) {

            _bombs = std::vector<std::vector<std::pair<Bomb, bomb_id_t>>>(gameOptions.bombTimer);
            generateHello();
        }

        void generateHello() {
            UDPMessage::clearBuffer();
            auto message = UDPMessage::getInstance();
            message << (uint8_t) 0
                    << _gameOptions.serverName
                    << _gameOptions.playerCount
                    << _gameOptions.sizeX
                    << _gameOptions.sizeY
                    << _gameOptions.gameLength
                    << _gameOptions.explosionRadius
                    << _gameOptions.bombTimer;
            _helloBuffer = UDPMessage::getBuffer();
        }

        bool isRunning() const {
            return _isRunning;
        }

        void disconnectPlayer(player_id_t playerId) {

        }

        void joinPlayer(player_id_t playerId, string playerName, string address) {
            std::cerr << "[debug] Got join player, id" << (int) playerId << " name "
                      << playerName << " address " << address << '\n';

            if (!isRunning() && !players.contains(playerId)) {
                static const uint8_t acceptedHeader = 1;
                Player newPlayer{std::move(playerName), std::move(address)};
                players.insert({playerId, newPlayer});

                UDPMessage::clearBuffer();
                UDPMessage::getInstance() << acceptedHeader
                                          << playerId
                                          << newPlayer.playerName
                                          << newPlayer.playerAddress;
                auto recentlyAcceptedPlayer = UDPMessage::getBuffer();

                _allAcceptedPlayers.insert(
                        _allAcceptedPlayers.end(),
                        recentlyAcceptedPlayer.begin(),
                        recentlyAcceptedPlayer.end());

                std::cerr << "[debug] send accepted to all\n";

                _server->sendToAll(recentlyAcceptedPlayer);

                std::cerr << "[debug] Ready " << players.size() << '/' << (int) _gameOptions.playerCount << '\n';

                if (players.size() == _gameOptions.playerCount) {
                    startGame();
                }
            }
        }

        void startGame() {
            _isRunning = true;
            UDPMessage::clearBuffer();
            UDPMessage::getInstance()
                    << (uint8_t) 2 // GameStarted
                    << players;

            _gameStarted = UDPMessage::getBuffer();

            std::cerr << "[debug] Game started.";

            _server->sendToAll(_gameStarted);

            newTurn();
        }


        void placeBomb(player_id_t playerId) {
            clearLastMove(playerId);
            _newBomb[playerId] = Bomb(_playerPositions[playerId], _gameOptions.bombTimer);
        }

        void placeBlock(player_id_t playerId) {
            clearLastMove(playerId);
            _newBlock[playerId] = _playerPositions[playerId];
        }

        void movePlayer(player_id_t playerId, uint8_t direction) {
            clearLastMove(playerId);
            Position newPosition = _playerPositions[playerId] + versors[direction];

            std::cerr << "[debug] Got move. New position: " << newPosition << "\n";

            if (isInside(newPosition))
                _newPlayerPosition[playerId] = newPosition;
        }

        bool isInside(const Position &position) const {
            return position.positionX < _gameOptions.sizeX &&
                   position.positionY < _gameOptions.sizeY;
        }

        void newTurn() {
            std::cerr << "[debug] New turn " << turn << "\n";
            events = 0;
            size_t explodingId = turn % _gameOptions.bombTimer;
            UDPMessage::clearBuffer();


            //UDPMessage::getInstance()
            //        << (uint8_t) 3              // turn header
            //        << turn
            //        << events;                 // Placeholder, until the number of events is determined.

            for (auto &bomb: _bombs[explodingId]) {
                calculateBombOutcome(bomb);
            }

            _bombs[explodingId].clear();

            calculateMovesOutcome();
            if (turn == 0) {
                generateBlocks();
            }

            //UDPMessage::loadAtIndex(sizeof(uint8_t) + sizeof(turn), events);

            //auto turnMessage = UDPMessage::getBuffer();
            //turns.insert(turns.end(), turnMessage.begin(), turnMessage.end());

            //_allTurns.push_back(_events);

            doCleanUp();

            //_server->sendToAll(turnMessage)
            UDPMessage::getInstance()
                    << (uint8_t) 3
                    << turn
                    << (list_size_t) _events.size();

            auto turnHeader = UDPMessage::getBuffer();
            _server->sendTurn(turnHeader, _events);

            _allTurns.push_back(turnHeader);
            for (auto &event: _events)
                _allTurns.push_back(event);

            ++turn;

            if (turn == _gameOptions.gameLength) {
                endGame();
            }
        }

        void doCleanUp() {
            for (const auto &dead: _playersDestroyer) {
                _alive.erase(dead);
                _playerPositions.erase(dead);
                ++_scores[dead];
            }

            for (const auto &block: _blocksDestroyed) {
                _blocks.erase(block);
            }

            clearMoves();
        }

        void calculateBombOutcome(std::pair<Bomb, bomb_id_t> &bomb) {
            ++events;

            std::unordered_set<player_id_t> playersDestroyed;
            std::unordered_set<Position> blocksDestroyed;

            for (const auto &versor: versors) {
                for (explosion_radius_t i = 0; i <= _gameOptions.explosionRadius; ++i) {
                    Position current = bomb.first.bombPosition + versor * i;
                    if (!isInside(current))
                        break;

                    for (auto &player: _playerPositions) {
                        if (player.second == current) {
                            playersDestroyed.insert(player.first);
                            _playersDestroyer.insert(player.first);
                        }
                    }

                    if (_blocks.contains(current)) {
                        blocksDestroyed.insert(current);
                        _blocks.insert(current);
                        break;
                    }
                }
            }

            UDPMessage::clearBuffer();
            UDPMessage::getInstance() << (uint8_t) 1 // Bomb exploded
                                      << bomb.second
                                      << playersDestroyed
                                      << blocksDestroyed;
            _events.push_back(UDPMessage::getBuffer());
        }

        void generateBlocks() {

        }

        void calculateMovesOutcome() {
            std::cerr << "[debug] Calculate moves outcome: " << players.size() << "\n";
            for (const auto &player: players) {
                auto playerId = player.first;
                std::cerr << "[debug] Now player " << (int) playerId << "\n";
                if (_alive.contains(playerId)) {
                    if (_newPlayerPosition.contains(playerId)) {
                        _playerPositions[playerId] = _newPlayerPosition[playerId];
                        loadPlayerMovedEvent(playerId);
                    } else if (_newBlock.contains(playerId)) {
                        auto placed = _newBlock[playerId];
                        if (!_blocks.contains(placed)) {
                            _blocks.insert(placed);
                            loadPlacedBlockEvent(placed);
                        }
                    } else if (_newBomb.contains(playerId)) {
                        loadBombPlacedEvent(_newBomb[playerId]);
                    }
                } else {
                    _newPlayerPosition[playerId] = randomPosition();
                    std::cerr << "[debug] Random position for player " << (int) playerId << " "
                              << _newPlayerPosition[playerId] << '\n';
                    _playerPositions[playerId] = _newPlayerPosition[playerId];
                    loadPlayerMovedEvent(playerId);
                    _alive.insert(playerId);
                }
            }
        }

        void loadPlayerMovedEvent(player_id_t playerId) {
            ++events;
            UDPMessage::clearBuffer();
            UDPMessage::getInstance() << (uint8_t) 2 << playerId << _newPlayerPosition[playerId];
            _events.push_back(UDPMessage::getBuffer());
        }

        void loadPlacedBlockEvent(const Position &position) {
            ++events;
            UDPMessage::clearBuffer();
            UDPMessage::getInstance() << (uint8_t) 3 << position;
            _events.push_back(UDPMessage::getBuffer());
        }

        void loadBombPlacedEvent(Bomb &bomb) {
            ++events;
            UDPMessage::clearBuffer();
            UDPMessage::getInstance() << (uint8_t) 0 << nextBombId << bomb.bombPosition;
            _bombs[turn % _gameOptions.bombTimer].push_back({bomb, nextBombId});
            _events.push_back(UDPMessage::getBuffer());

            ++nextBombId;
        }

        const std::vector<uint8_t> &helloMessage() const {
            return _helloBuffer;
        }

        void clearMoves() {
            _newPlayerPosition.clear();
            _newBomb.clear();
            _newBlock.clear();
            _playersDestroyer.clear();
            _blocksDestroyed.clear();
            _events.clear();
        }

        void clearLastMove(player_id_t playerId) {
            _newPlayerPosition.erase(playerId);
            _newBomb.erase(playerId);
            _newBlock.erase(playerId);
        }

        void endGame() {
            UDPMessage::clearBuffer();
            UDPMessage::getInstance() << (uint8_t) 4 << _scores;
            _server->sendToAll(UDPMessage::getBuffer());
            _isRunning = false;
            players.clear();
            _playerPositions.clear();
            _blocks.clear();
            _scores.clear();
            _allAcceptedPlayers.clear();
            _gameStarted.clear();
            _allTurns.clear();
            for (auto &bombBucket: _bombs)
                bombBucket.clear();
            _alive.clear();
            turns.clear();
            clearMoves();
            turn = 0;
        }

        GameOptions _gameOptions;
        bool _isRunning;
        players_t players;
        std::vector<uint8_t> _helloBuffer;
        std::vector<uint8_t> _allAcceptedPlayers;
        std::vector<uint8_t> _gameStarted;
        std::shared_ptr<IServer> _server;
        std::vector<std::vector<std::pair<Bomb, bomb_id_t>>> _bombs;
        uint16_t turn = 0;
        list_size_t events = 0;

        bomb_id_t nextBombId = 0;
        std::unordered_map<player_id_t, Position> _playerPositions;
        std::unordered_set<Position> _blocks;
        std::unordered_map<player_id_t, score_t> _scores;

        std::unordered_map<player_id_t, Position> _newPlayerPosition;
        std::unordered_map<player_id_t, Bomb> _newBomb;
        std::unordered_map<player_id_t, Position> _newBlock;
        std::unordered_set<player_id_t> _alive;

        std::unordered_set<player_id_t> _playersDestroyer;
        std::unordered_set<Position> _blocksDestroyed;
    };
}

#endif //ROBOTS_CLIENT_GAME_H