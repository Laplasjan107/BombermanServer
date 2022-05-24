//
// Created by Piotr Kamiński on 23/05/2022.
//

#ifndef BOMBERMANSERVER_GAMESTATUS_H
#define BOMBERMANSERVER_GAMESTATUS_H

#include "types.h"
#include "common.h"
#include "Player.h"
#include "Bomb.h"
#include "messages/client-gui/UDPMessage.h"
#include <unordered_set>
#include <vector>
#include "event/events.h"

namespace bomberman {
    struct GameStatus {
        bool running = false;

        string serverName{};
        players_count_t playersCount{};
        board_size_t sizeX{};
        board_size_t sizeY{};
        explosion_radius_t explosionRadius{};
        game_length_t bombTimer{};
        game_length_t gameLength{};
        game_length_t turn{};
        players_t players;
        players_position_t positions;
        std::unordered_set<Position> blocks;
        std::unordered_map<bomb_id_t, Bomb> bombs;
        std::unordered_set<Position> explosions;
        //std::vector<Position> explosions;
        std::unordered_map<player_id_t, score_t> scores;
        std::unordered_set<player_id_t> destroyedPlayers;

    public:
        explicit GameStatus(const HelloMessage &hello) :
            serverName(hello.mapSettings.serverName),
            playersCount(hello.mapSettings.playersCount),
            explosionRadius(hello.mapSettings.explosionRadius),
            sizeX(hello.mapSettings.sizeX),
            sizeY(hello.mapSettings.sizeY),
            gameLength(hello.mapSettings.gameLength),
            turn()
        {
        }

        GameStatus() = default;

        void initializeGame(const HelloMessage &hello) {
            serverName = hello.mapSettings.serverName;
            sizeX = hello.mapSettings.sizeX;
            sizeY = hello.mapSettings.sizeY;
            gameLength = hello.mapSettings.gameLength;
            explosionRadius = hello.mapSettings.explosionRadius;
            turn = 0;
        }

        void newTurn() {
            destroyedPlayers = std::unordered_set<player_id_t> {};
            std::cerr << "EXPLOSION CLEARED " << explosions.size() << '\n';
            explosions = std::unordered_set<Position>{};
            for (auto &bomb: bombs) {
                bomb.second.timer -= 1;
            }
        }

        void newPlayer(const AcceptedPlayerMessage &accepted) {
            players.insert({accepted.playerId, accepted.player});
            scores.insert({accepted.playerId, 0});
        }

        void startGame(GameStartedMessage &started) {
            running = true;
            players = std::move(started.activePlayers);
        }

        void placeBomb(const BombPlacedEvent& bombPlaced) {
            bombs.insert({bombPlaced.bombId, Bomb {bombPlaced.position, bombTimer}});
        }

        bool isInside(const Position &position) {
            return (position.positionX < sizeX) && (position.positionY < sizeY) &&
                    (position.positionX >= 0) && (position.positionY >= 0);
        }

        void renderExplosion(const Position &position) {
            static const std::vector<Position> directions {Position {1, 0},
                                                           Position {0, 1},
                                                           Position {(board_size_t) -1, 0},
                                                           Position {0, (board_size_t) -1}};

            for (auto &versor: directions) {
                std::cerr << "RENDERING EXPL: " << explosions.size() << "rad = " << explosionRadius << '\n';
                for (explosion_radius_t i = 0; i <= explosionRadius; ++i) {
                    Position currentPosition = position + versor * i;
                    if (!isInside(currentPosition))
                        break;

                    std::cerr << "checking " << currentPosition <<'\n';
                    explosions.insert(currentPosition);
                    if (blocks.contains(currentPosition))
                        break;
                }
            }
        }

        void explodeBomb(const BombExplodedEvent &bombExplodedEvent) {
            Position explosion = bombs[bombExplodedEvent.bombId].bombPosition;
            renderExplosion(explosion);
            bombs.erase(bombExplodedEvent.bombId);
            for (auto block: bombExplodedEvent.blocksDestroyed) {
                blocks.erase(block);
            }

            for (auto player: bombExplodedEvent.robotsDestroyed) {
                if (!destroyedPlayers.contains(player)) {
                    destroyedPlayers.insert(player);
                    scores[player] += 1;
                }
            }
        }

        void placeBlock(Position blockPosition) {
            blocks.insert(blockPosition);
        }

        void endGame(const GameEndedMessage &gameEnded) {
            using namespace std;
            std::cerr << "EXPLOSION CLEARED " << explosions.size() << '\n';

            running = false;
            blocks = unordered_set<Position>{};
            bombs = unordered_map<bomb_id_t, Bomb> {};
            explosions = unordered_set<Position> {};
            scores = unordered_map<player_id_t, score_t> {};
        }

        GameStatus(string name, board_size_t x, board_size_t y, game_length_t l) :
            serverName(std::move(name)),
            sizeX(x),
            sizeY(y),
            gameLength(l),
            turn(0)
        {
            scores.insert({1, 1});
        }

        void writeToUDP() {
            UDPMessage::clearBuffer();
            if (running) {
                UDPMessage::loadNumber(static_cast<uint8_t>(DrawMessageType::Game));
                UDPMessage::loadString(serverName);
                UDPMessage::loadNumber(sizeX);
                UDPMessage::loadNumber(sizeY);
                UDPMessage::loadNumber(gameLength);
                UDPMessage::loadNumber(turn);
                UDPMessage::loadPlayersMap(players);
                UDPMessage::loadPlayersPositions(positions);
                UDPMessage::loadPositionsList(blocks);
                UDPMessage::loadBombsList(bombs);
                UDPMessage::loadPositionsList(explosions);
                UDPMessage::loadScores(scores);
            }
            else {
                UDPMessage::loadNumber(static_cast<uint8_t>(DrawMessageType::Lobby));
                UDPMessage::loadString(serverName);
                UDPMessage::loadNumber(playersCount);
                UDPMessage::loadNumber(sizeX);
                UDPMessage::loadNumber(sizeY);
                UDPMessage::loadNumber(gameLength);
                UDPMessage::loadNumber(explosionRadius);
                UDPMessage::loadNumber(bombTimer);
                UDPMessage::loadPlayersMap(players);
            }
        }
    };
}

#endif //BOMBERMANSERVER_GAMESTATUS_H
