//
// Created by Piotr Kamiński on 03/06/2022.
//

#ifndef ROBOTS_CLIENT_SERVEROPTIONS_H
#define ROBOTS_CLIENT_SERVEROPTIONS_H

//#include <boost/program_options.hpp>
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
        int32_t seed;

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

        GameOptions([[maybe_unused]]int argumentsCount, [[maybe_unused]] char *argumentsTable[]) {
/*            namespace po = boost::program_options;

            po::options_description description("Options parser");
            description.add_options()
                    ("help,h", "Help request")
                    ("display-address,d", po::value<std::string>()->required(), "Port number")
                    ("player-name,n", po::value<std::string>()->required(), "Player name")
                    ("port,p", po::value<uint16_t>()->required(), "Port number")
                    ("server-address,s", po::value<std::string>()->required(), "Server address");

            po::variables_map programVariables;
            po::store(po::command_line_parser(argumentsCount, argumentsTable).options(description).run(),
                      programVariables);
            if (programVariables.count("help"))
                throw;

            po::notify(programVariables);
            displayAddress = programVariables["display-address"].as<std::string>();
            serverAddress = programVariables["server-address"].as<std::string>();
            playerName = programVariables["player-name"].as<std::string>();
            port = programVariables["port"].as<uint16_t>(); */
        }
    };
}

#endif //ROBOTS_CLIENT_SERVEROPTIONS_H
