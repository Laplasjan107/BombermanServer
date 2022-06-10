//
// Created by Piotr Kamiński on 03/06/2022.
//

#ifndef ROBOTS_CLIENT_SERVEROPTIONS_H
#define ROBOTS_CLIENT_SERVEROPTIONS_H

#include <boost/program_options.hpp>
#include "types.h"

namespace bomberman {
    struct GameOptions {
        std::string serverName;
        players_count_t playerCount;
        board_size_t sizeX;
        board_size_t sizeY;
        game_length_t gameLength;
        explosion_radius_t explosionRadius;
        bomb_timer_t bombTimer;
        uint64_t turnTimer;
        uint16_t port;
        uint32_t seed;
        uint64_t turnDuration;
        uint16_t initialBlocks;

        GameOptions() {
            serverName = "to_jest_serwer";
            playerCount = 2;
            sizeX = 15;
            sizeY = 15;
            gameLength = 500;
            explosionRadius = 3;
            bombTimer = 4;
            turnTimer = 200;
            port = 12345;
            seed = 3456;
        }

        GameOptions(int argumentsCount, char *argumentsTable[]) {
            namespace po = boost::program_options;

            po::options_description description("Options parser");
            description.add_options()
                    ("help,h", "Help request")
                    ("bomb-timer,b", po::value<bomb_timer_t>()->required(), "Bomb timer (rounds)")
                    ("players-count,c", po::value<players_count_t>()->required(), "Number of players")
                    ("turn-duration,d", po::value<uint64_t>()->required(), "Duration of one turn (ms)")
                    ("explosion-radius,e", po::value<board_size_t>()->required(), "Bomb explosion radius")
                    ("initial-blocks,k", po::value<uint16_t>()->required(),
                            "Number of randomly generated blocks at the start")
                    ("game-length,l", po::value<game_length_t>()->required(), "Number of turns in game")
                    ("server-name,n", po::value<std::string>()->required(), "Server name")
                    ("port,p", po::value<uint16_t>()->required(), "Port, on which the server listens")
                    ("seed,s", po::value<uint32_t>()->default_value(1997)->required(), "Seed for the random numbers generator")
                    ("size-x,x", po::value<board_size_t>()->required(), "Horizontal length of the board")
                    ("size-y,y", po::value<board_size_t>()->required(), "Vertical length of the board");

            po::variables_map programVariables;
            po::store(po::command_line_parser(argumentsCount, argumentsTable).options(description).run(),
                      programVariables);
            if (programVariables.count("help"))
                throw;

            po::notify(programVariables);
            bombTimer = programVariables["bomb-timer"].as<bomb_timer_t>();
            playerCount = programVariables["players-count"].as<players_count_t>();
            turnDuration = programVariables["turn-duration"].as<uint64_t>();
            explosionRadius = programVariables["explosion-radius"].as<board_size_t>();
            initialBlocks = programVariables["initial-blocks"].as<uint16_t>();
            gameLength = programVariables["turn-duration"].as<game_length_t>();
            serverName = programVariables["server-name"].as<std::string>();
            port = programVariables["port"].as<uint16_t>();
            seed = programVariables["seed"].as<uint32_t>();
            sizeX = programVariables["size-x"].as<board_size_t>();
            sizeY = programVariables["size-y"].as<board_size_t>();
        }
    };
}

#endif //ROBOTS_CLIENT_SERVEROPTIONS_H
