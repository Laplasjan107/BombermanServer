//
// Created by Piotr Kamiński on 23/05/2022.
//

#ifndef BOMBERMANSERVER_CLIENTOPTIONS_H
#define BOMBERMANSERVER_CLIENTOPTIONS_H

#include "common.h"
#include "types.h"

namespace bomberman {
    class ClientOptions {
    public:
        std::string     playerName;
        uint16_t        port;
        std::string     serverAddress;
        std::string     displayAddress;

        struct HelpException : public std::invalid_argument {
            explicit HelpException (const std::string &description) : invalid_argument(description) { }

            [[nodiscard]] const char *what () const noexcept override {
                return "Help message requested";
            }
        };

        ClientOptions(int argumentsCount, char *argumentsTable[]) {
            /*
            po::options_description description("Options parser");
            description.add_options()
                    ("help,h", "Help request")
                    ("display-address,d", po::value<std::string>()->required(), "Port number")
                    ("player-name,n", po::value<std::string>()->required(), "Player name")
                    ("port,p", po::value<uint16_t>()->required(), "Port number")
                    ("server-address,s", po::value<std::string>()->required(), "Server address");

            po::variables_map programVariables;
            po::store(po::command_line_parser(argumentsCount, argumentsTable).options(description).run(), programVariables);
            if (programVariables.count("help"))
                throw HelpException("Asked for help message");

            po::notify(programVariables);
            displayAddress  = programVariables["display-address"].as<std::string>();
            serverAddress   = programVariables["server-address"].as<std::string>();
            playerName      = programVariables["player-name"].as<std::string>();
            port            = programVariables["port"].as<uint16_t>();
             */
        }
    };
}

#endif //BOMBERMANSERVER_CLIENTOPTIONS_H
