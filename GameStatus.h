//
// Created by Piotr Kamiński on 23/05/2022.
//

#ifndef BOMBERMANSERVER_GAMESTATUS_H
#define BOMBERMANSERVER_GAMESTATUS_H

#include "types.h"
#include "common.h"
#include "Player.h"
#include "Bomb.h"
#include <vector>

namespace bomberman {
    struct GameStatus {
        string serverName;
        board_size_t sizeX;
        board_size_t sizeY;
        game_length_t gameLength;
        game_length_t turn;
        players_t players;
        players_position_t positions;
        std::vector<Position> blocks;
        std::vector<Bomb> bombs;
        std::vector<Position> explosions;
        std::unordered_map<player_id_t, score_t> scores;


    public:
        explicit GameStatus(const HelloMessage &hello) :
            serverName(hello.mapSettings.serverName),
            sizeX(hello.mapSettings.sizeX),
            sizeY(hello.mapSettings.sizeY),
            gameLength(hello.mapSettings.gameLength),
            turn()
        {
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


    };
}

#endif //BOMBERMANSERVER_GAMESTATUS_H
