#include "client.h"

namespace po = boost::program_options;
using boost::asio::ip::tcp;

static const constexpr char helpMessage[] = "This is Bomberman game client.\n"
                     "Flags:\n"
                     "    -h, --help\n"
                     "    -d, --display-address (Required)\n"
                     "    -n, --player-name (Required)\n"
                     "    -p, --port (Required)\n"
                     "    -s, --server-address (Required)";

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

std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

int main(int argc, char *argv[]) {
    try {
        ClientOptions client {argc, argv};

        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("students.mimuw.edu.pl:", "daytime");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        for (;;)
        {
            boost::array<char, 128> buf {};
            boost::system::error_code error;

            auto len = socket.read_some(boost::asio::buffer(buf), error);
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            std::cout.write(buf.data(), (std::streamsize) len);
        }

    }
    catch (ClientOptions::HelpException &exception) {
        std::cout << helpMessage << std::endl;
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
    }

    return 0;
}
