#include "client.h"

namespace po = boost::program_options;
using boost::asio::ip::tcp;

char helpMessage[] = "This is Bomberman game client.\n"
                     "Flags:\n"
                     "    -h, --help\n"
                     "    -d, --display-address (Required)\n"
                     "    -n, --player-name (Required)\n"
                     "    -p, --port (Required)\n"
                     "    -s, --server-address (Required)";

class JoinMessage : IMessage {
public:
    JoinMessage() {

    }
};

struct HelloMessage : IMessage {

};

struct AcceptedPlayerMessage : IMessage {

};

struct GameStartedMessage : IMessage {

};

struct TurnMessage : IMessage {

};

struct GameEndedMessage : IMessage {

};

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
    }
};

int main(int argc, char *argv[]) {
    try {
        ClientOptions client {argc, argv};
    }
    catch (ClientOptions::HelpException &exception) {
        std::cout << helpMessage << std::endl;
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
    }

    return 0;
}
