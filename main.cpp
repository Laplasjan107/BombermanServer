#include <iostream>
#include <string>
#include <exception>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using boost::asio::ip::tcp;

enum ClientMessageType {
    Join            = 0,
    PlaceBomb       = 1,
    PlaceBlock      = 2,
    Move            = 3,
};

enum Direction {
    Up              = 0,
    Right           = 1,
    Down            = 2,
    Left            = 3,
};

class IMessage {
public:
    virtual ~IMessage() {}
};

class JoinMessage : IMessage {
public:
    JoinMessage() {

    }
};

class ClientOptions {
    static bool areAllMandatoryFlagsSet(po::variables_map &programVariables) {
        return programVariables.count("display-address") && programVariables.count("player-name") &&
                programVariables.count("port") && programVariables.count("server-address");
    }

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
        po::options_description description("Options parser");
        description.add_options()
                ("display-address,d", po::value<std::string>(), "Port number")
                ("help,h", "Help request")
                ("player-name,n", po::value<std::string>(), "Player name")
                ("port,p", po::value<uint16_t>(), "Port number")
                ("server-address,s", po::value<std::string>(), "Server address");

        po::variables_map programVariables;
        po::store(po::command_line_parser(argumentsCount, argumentsTable).options(description).run(), programVariables);
        po::notify(programVariables);

        if (programVariables.count("help"))
            throw HelpException("Asked for help message");

        if (!areAllMandatoryFlagsSet(programVariables))
            throw std::invalid_argument("Flags: display-address, player-name, port, server-address are mandatory");

        displayAddress  = programVariables["display-address"].as<std::string>();
        serverAddress   = programVariables["server-address"].as<std::string>();
        playerName      = programVariables["player-name"].as<std::string>();
        port            = programVariables["port"].as<uint16_t>();
    }
};

int main(int argc, char *argv[]) {
    try {
        ClientOptions client {argc, argv};
    }
    catch (ClientOptions::HelpException &exception) {
        std::cout << "This is Bomberman game client.\n"
                     "Flags:\n"
                     "    -h, --help\n"
                     "    -d, --display-address (Required)\n"
                     "    -n, --player-name (Required)\n"
                     "    -p, --port (Required)\n"
                     "    -s, --server-address (Required)\n";
    }
    catch (std::exception &exception) {
        std::cerr << exception.what();
    }

    return 0;
}
