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
#include <atomic>
#include "event/events.h"

namespace bomberman {
    class GameStatus {
        MapSettings mapSettings;
        std::atomic<bool> running {false};

        game_length_t turn{};
        players_t players;
        players_position_t positions;
        std::unordered_set<Position> blocks;
        std::unordered_map<bomb_id_t, Bomb> bombs;
        std::unordered_set<Position> explosions;
        std::unordered_map<player_id_t, score_t> scores;
        std::unordered_set<player_id_t> destroyedPlayers;

        bool isInside(const Position &position) const {
            return (position.positionX < mapSettings.sizeX) && (position.positionY < mapSettings.sizeY);
        }

        void renderExplosion(const Position &position) {
            static const std::vector<Position> directions{Position{1, 0},
                                                          Position{0, 1},
                                                          Position{(board_size_t) -1, 0},
                                                          Position{0, (board_size_t) -1}};

            for (auto &versor: directions) {
                for (explosion_radius_t i = 0; i <= mapSettings.explosionRadius; ++i) {
                    Position currentPosition = position + versor * i;
                    if (!isInside(currentPosition))
                        break;

                    explosions.insert(currentPosition);
                    if (blocks.contains(currentPosition) || explodedBlocks.contains(currentPosition))
                        break;
                }
            }
        }

        void writeGameToUDP() {
            std::cerr << "[debug] Writing game to UDP:\n";
            std::cerr << mapSettings.serverName << std::endl;
            for (auto &player: players) {
                std::cerr << (int) player.first << " " << player.second.playerName << player.second.playerAddress << "\n";
            }

            UDPMessage::getInstance()
                    << static_cast<uint8_t>(DrawMessageType::Game)
                    << mapSettings.serverName
                    << mapSettings.sizeX
                    << mapSettings.sizeY
                    << mapSettings.gameLength
                    << turn
                    << players
                    << positions
                    << blocks
                    << bombs
                    << explosions
                    << scores;
        }

        void writeLobbyToUDP() {
            UDPMessage::getInstance()
                    << static_cast<uint8_t>(DrawMessageType::Lobby)
                    << mapSettings.serverName
                    << mapSettings.playersCount
                    << mapSettings.sizeX
                    << mapSettings.sizeY
                    << mapSettings.gameLength
                    << mapSettings.explosionRadius
                    << mapSettings.bombTimer
                    << players;
        }

    public:
        GameStatus() = default;

        void initializeGame(const HelloMessage &hello) {
            running = false;
            mapSettings = hello.mapSettings;
        }

        std::unordered_set<Position> explodedBlocks;

        void newTurn(game_length_t turnNumber) {
            turn = turnNumber;
            destroyedPlayers = std::unordered_set<player_id_t>{};
            explosions = std::unordered_set<Position>{};
            explodedBlocks = std::unordered_set<Position>{};
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
            for (const auto &player: started.activePlayers)
                scores.insert({player.first, 0});
            players = std::move(started.activePlayers);
        }

        void placeBomb(const BombPlacedEvent &bombPlaced) {
            bombs.insert({bombPlaced.bombId, Bomb{bombPlaced.position, mapSettings.bombTimer}});
        }

        void explodeBomb(const BombExplodedEvent &bombExplodedEvent) {
            Position explosion = bombs[bombExplodedEvent.bombId].bombPosition;
            renderExplosion(explosion);
            bombs.erase(bombExplodedEvent.bombId);
            for (auto block: bombExplodedEvent.blocksDestroyed) {
                blocks.erase(block);
                explodedBlocks.insert(block);
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

        void endGame([[maybe_unused]] const GameEndedMessage &gameEnded) {
            using namespace std;

            running = false;
            blocks = unordered_set<Position>{};
            bombs = unordered_map<bomb_id_t, Bomb>{};
            explosions = unordered_set<Position>{};
            scores = unordered_map<player_id_t, score_t>{};
            players = unordered_map<player_id_t, Player>{};
        }

        bool isRunning() const {
            return running;
        }

        void movePlayer(player_id_t playerId, const Position &position) {
            positions[playerId] = position;
        }

        void writeToUDP() {
            UDPMessage::clearBuffer();
            if (running) {
                writeGameToUDP();
            } else {
                writeLobbyToUDP();
            }
        }
    };
}

#endif //BOMBERMANSERVER_GAMESTATUS_H
