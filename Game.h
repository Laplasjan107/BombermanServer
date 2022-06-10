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
#include "messages/client-gui/Message.h"
#include "ServerOptions.h"
#include "Bomb.h"
#include "messages/server-client/ServerMessageType.h"

namespace bomberman {
    /*  Main gaming logic.
     *  Server orders new turn start, while sessions request moves.
     */
    class Game {
    public:
        std::vector<uint8_t> GameStarted;
        std::vector<uint8_t> AllAcceptedPlayers;
        turn_message AllTurns;

        explicit Game(ServerOptions &gameOptions, std::shared_ptr<IServer> server) :
                _gameOptions(gameOptions),
                _isRunning(false),
                _server(std::move(server)) {

            _bombs = std::vector<std::vector<std::pair<Bomb, bomb_id_t>>>(gameOptions.bombTimer);
            generateHello();
            _lastRandom = gameOptions.seed;
        }

        bool isRunning() const {
            return _isRunning;
        }

        void joinPlayer(int sessionId, string playerName, string address) {
            if (!isRunning() && !_session_to_player.contains(sessionId)) {
                _session_to_player[sessionId] = (uint8_t) _session_to_player.size();
                player_id_t playerId = _session_to_player[sessionId];

                Player newPlayer{std::move(playerName), std::move(address)};
                players.insert({playerId, newPlayer});
                _playerIds.insert(playerId);

                Message::clearBuffer();
                Message::getInstance() << ServerMessageType::AcceptedPlayer
                                       << playerId
                                       << newPlayer.playerName
                                       << newPlayer.playerAddress;
                auto recentlyAcceptedPlayer = Message::getBuffer();

                AllAcceptedPlayers.insert(
                        AllAcceptedPlayers.end(),
                        recentlyAcceptedPlayer.begin(),
                        recentlyAcceptedPlayer.end());

                _server->sendToAll(recentlyAcceptedPlayer);
                if (players.size() == _gameOptions.playerCount) {
                    startGame();
                }
            }
        }

        void startGame() {
            _isRunning = true;
            Message::clearBuffer();
            Message::getInstance()
                    << ServerMessageType::GameStarted
                    << players;

            GameStarted = Message::getBuffer();
            _server->sendToAll(GameStarted);

            newTurn();
        }


        void placeBomb(int sessionId) {
            if (isRunning() && _session_to_player.contains(sessionId)) {
                player_id_t playerId = _session_to_player[sessionId];
                clearLastMove(playerId);
                _newBomb[playerId] = Bomb(_playerPositions[playerId], _gameOptions.bombTimer);
            }
        }

        void placeBlock(int sessionId) {
            if (isRunning() && _session_to_player.contains(sessionId)) {
                player_id_t playerId = _session_to_player[sessionId];
                clearLastMove(playerId);
                _newBlock[playerId] = _playerPositions[playerId];
            }
        }

        void movePlayer(int sessionId, uint8_t direction) {
            if (isRunning() && _session_to_player.contains(sessionId)) {
                player_id_t playerId = _session_to_player[sessionId];
                clearLastMove(playerId);
                Position newPosition = _playerPositions[playerId] + versors[direction];

                if (isInside(newPosition))
                    _newPlayerPosition[playerId] = newPosition;
            }
        }

        void newTurn() {
            events = 0;
            size_t explodingId = turn % _gameOptions.bombTimer;
            Message::clearBuffer();

            for (auto &bomb: _bombs[explodingId]) {
                calculateBombOutcome(bomb);
            }
            _bombs[explodingId].clear();

            calculateMovesOutcome();
            if (turn == 0) {
                generateBlocks();
            }

            Message::getInstance()
                    << ServerMessageType::Turn
                    << turn
                    << (list_size_t) _events.size();

            auto turnHeader = Message::getBuffer();
            _server->sendTurn(turnHeader, _events);

            AllTurns.push_back(turnHeader);
            for (auto &event: _events)
                AllTurns.push_back(event);

            doCleanUp();

            if (turn == _gameOptions.gameLength) {
                endGame();
            }
            ++turn;
        }

    private:
        std::unordered_map<int, player_id_t> _session_to_player;
        turn_message _events;
        size_t _lastRandom;

        size_t random() {
            _lastRandom *= 48271;
            _lastRandom %= 2147483647;
            return _lastRandom;
        }

        Position randomPosition() {
            return {static_cast<board_size_t>(random() % _gameOptions.sizeX),
                    static_cast<board_size_t>(random() % _gameOptions.sizeY)};
        }

        std::vector<uint8_t> turns;
        static const constexpr Position versors[numberOfDirections] =
                {{0,                 1},
                 {1,                 0},
                 {0,                 (board_size_t) -1},
                 {(board_size_t) -1, 0},
                };

        void generateHello() {
            Message::clearBuffer();
            auto message = Message::getInstance();
            message << ServerMessageType::Hello
                    << _gameOptions.serverName
                    << _gameOptions.playerCount
                    << _gameOptions.sizeX
                    << _gameOptions.sizeY
                    << _gameOptions.gameLength
                    << _gameOptions.explosionRadius
                    << _gameOptions.bombTimer;
            _helloBuffer = Message::getBuffer();
        }

        bool isInside(const Position &position) const {
            return position.positionX < _gameOptions.sizeX &&
                   position.positionY < _gameOptions.sizeY;
        }

        void doCleanUp() {
            for (const auto &dead: _playersDestroyer) {
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
                            _alive.erase(player.first);
                        }
                    }

                    if (_blocks.contains(current)) {
                        blocksDestroyed.insert(current);
                        _blocksDestroyed.insert(current);
                        break;
                    }
                }
            }

            Message::clearBuffer();
            Message::getInstance() << EventType::BombExploded
                                   << bomb.second
                                   << playersDestroyed
                                   << blocksDestroyed;
            _events.push_back(Message::getBuffer());
        }

        void generateBlocks() {
            for (int i = 0; i < _gameOptions.initialBlocks; ++i) {
                auto newBlock = randomPosition();
                if (!_blocks.contains(newBlock)) {
                    _blocks.insert(newBlock);
                    loadPlacedBlockEvent(newBlock);
                }
            }
        }

        void calculateMovesOutcome() {
            for (const auto playerId: _playerIds) {
                if (_alive.contains(playerId)) {
                    if (_newPlayerPosition.contains(playerId)) {
                        if (isInside(_newPlayerPosition[playerId]) &&
                            !_blocks.contains(_newPlayerPosition[playerId])) {
                            _playerPositions[playerId] = _newPlayerPosition[playerId];
                            loadPlayerMovedEvent(playerId);
                        }
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
                    _playerPositions[playerId] = _newPlayerPosition[playerId];
                    loadPlayerMovedEvent(playerId);
                    _alive.insert(playerId);
                }
            }
        }

        void loadPlayerMovedEvent(player_id_t playerId) {
            ++events;
            Message::clearBuffer();
            Message::getInstance() << EventType::PlayerMoved << playerId << _newPlayerPosition[playerId];
            _events.push_back(Message::getBuffer());
        }

        void loadPlacedBlockEvent(const Position &position) {
            ++events;
            Message::clearBuffer();
            Message::getInstance() << EventType::BlockPlaced << position;
            _events.push_back(Message::getBuffer());
        }

        void loadBombPlacedEvent(Bomb &bomb) {
            ++events;
            Message::clearBuffer();
            Message::getInstance() << EventType::BombPlaced << nextBombId << bomb.bombPosition;
            _bombs[turn % _gameOptions.bombTimer].push_back({bomb, nextBombId});
            _events.push_back(Message::getBuffer());

            ++nextBombId;
        }

    public:
        const std::vector<uint8_t> &helloMessage() const {
            return _helloBuffer;
        }

    private:
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
            Message::clearBuffer();
            Message::getInstance() << ServerMessageType::GameEnded << _scores;
            _server->sendToAll(Message::getBuffer());
            _isRunning = false;
            players.clear();
            _playerPositions.clear();
            _blocks.clear();
            _scores.clear();
            AllAcceptedPlayers.clear();
            GameStarted.clear();
            AllTurns.clear();
            _playerIds.clear();
            _session_to_player.clear();
            for (auto &bombBucket: _bombs)
                bombBucket.clear();
            _alive.clear();
            turns.clear();
            clearMoves();
            turn = 0;
        }

        ServerOptions _gameOptions;
        bool _isRunning;
        players_t players;
        std::set<player_id_t> _playerIds;
        std::vector<uint8_t> _helloBuffer;


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
