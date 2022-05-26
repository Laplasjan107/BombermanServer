//
// Created by Piotr Kamiński on 23/05/2022.
//

#ifndef BOMBERMANSERVER_CLIENTOPTIONS_H
#define BOMBERMANSERVER_CLIENTOPTIONS_H

#include <boost/program_options.hpp>
#include "common.h"
#include "types.h"

namespace bomberman {
    struct ClientOptions {
        std::string playerName;
        uint16_t port;
        std::string serverAddress;
        std::string displayAddress;

        std::string serverIP;
        std::string serverPort;

        std::string guiIP;
        std::string guiPort;

        struct HelpException : public std::invalid_argument {
            explicit HelpException (const std::string &description) : invalid_argument(description) { }

            [[nodiscard]] const char *what () const noexcept override {
                return "Help message requested";
            }
        };

        ClientOptions(int argumentsCount, char *argumentsTable[]) {
            using namespace boost::program_options;

            options_description description("Options parser");
            description.add_options()
                    ("help,h", "Help request")
                    ("display-address,d", value<std::string>()->required(), "Port number")
                    ("player-name,n", value<std::string>()->required(), "Player name")
                    ("port,p", value<uint16_t>()->required(), "Port number")
                    ("server-address,s", value<std::string>()->required(), "Server address");

            variables_map programVariables;
            store(command_line_parser(argumentsCount, argumentsTable).options(description).run(), programVariables);
            if (programVariables.count("help"))
                throw HelpException("Asked for help message");

            notify(programVariables);
            displayAddress  = programVariables["display-address"].as<std::string>();
            serverAddress   = programVariables["server-address"].as<std::string>();
            playerName      = programVariables["player-name"].as<std::string>();
            port            = programVariables["port"].as<uint16_t>();

            auto slicedServer = sliceAddress(serverAddress);
            auto slicedDisplay = sliceAddress(displayAddress);

            serverIP = slicedServer.first;
            serverPort = slicedServer.second;
            guiIP = slicedDisplay.first;
            guiPort = slicedDisplay.second;
        }

    private:
        [[nodiscard]] static std::pair<std::string, std::string> sliceAddress(const std::string &address) {
            auto iterator = address.size() - 1;
            while (iterator > 0) {
                if (address[iterator] == ':')
                    break;
                --iterator;
            }
            if (iterator == 0 || iterator == address.size() - 1)
                throw std::invalid_argument("Not a valid address");

            return {address.substr(0, iterator), address.substr(iterator + 1, address.size() - 1)};
        }
    };
}

#endif //BOMBERMANSERVER_CLIENTOPTIONS_H
