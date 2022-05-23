#include "client.h"

//namespace po = boost::program_options;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using namespace bomberman;

static const constexpr char helpMessage[] = "This is Bomberman game client.\n"
                     "Flags:\n"
                     "    -h, --help\n"
                     "    -d, --display-address (Required)\n"
                     "    -n, --player-name (Required)\n"
                     "    -p, --port (Required)\n"
                     "    -s, --server-address (Required)";

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
        port            = programVariables["port"].as<uint16_t>(); */
    }
};

int main(int argc, char *argv[]) {
    try {
        ClientOptions client {argc, argv};

        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("localhost", "14006");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        //udp::resolver res(io_context);
        //auto endpt = res.resolve("localhost", "14008");
        //boost::asio::ip::udp::endpoint destination(
                //boost::asio::ip::address::from_string("127.0.0.1"), 14008);
                //udp::socket udpSocket(io_context);
        //udpSocket.open(boost::asio::ip::udp::v6());
        udp::socket udpSocket(io_context, udp::endpoint(udp::v6(), 0));
        udp::resolver res(io_context);
        udp::resolver::query query(udp::v6(), "localhost", "12345");
        udp::resolver::iterator iter = res.resolve(query);
        udp::endpoint e = *iter;


        //boost::asio::connect(udpSocket, endpt);
        //auto endpt = boost::asio::ip::udp::endpoint{boost::asio::ip::make_address("127.0.0.1"), 14008};
        //udpSocket.send_to(boost::asio::buffer(message), endpt);

        for (int i = 0; i < 5; ++i)
        {
            MapSettings settings;
            settings.bombTimer = 1;
            settings.explosionRadius = 1;
            settings.gameLength = 5;
            settings.serverName = "AHH";
            settings.playersCount = 1;
            settings.sizeX = 5;
            settings.sizeY = 5;
            players_t pl;
            pl.insert({10, Player("aaa", "27.0.0.1:44152")});
            LobbyMessage lobb {settings, pl};
            lobb.sendAndClear(udpSocket, e);
            GameStatus status{"SER", 4, 5, 5};
            UDPMessage::loadGameStatus(status);
            UDPMessage::sendAndClear(udpSocket, e);

            boost::array<char, 128> buf {};
            boost::system::error_code error;

            std::string m = "xxaa";
            m[0] = 0; m[1] = 2;
            boost::asio::write(socket, boost::asio::buffer(m, 4));

            ServerMessageType messageType;
            boost::asio::read(socket, boost::asio::buffer(&messageType, sizeof(messageType)));
            std::cout << "New message\n";
            switch (messageType) {
                case ServerMessageType::Hello:
                {
                    auto hello = HelloMessage(socket);
                    hello.print();
                }
                    break;
                case ServerMessageType::AcceptedPlayer:
                {
                    auto accepted = AcceptedPlayerMessage(socket);
                    accepted.print();
                }
                    break;
                case ServerMessageType::GameStarted:
                {
                    auto started = GameStartedMessage(socket);
                    started.print();
                }
                    break;
                case ServerMessageType::Turn:
                    break;
                case ServerMessageType::GameEnded:
                    break;
            }

            /* if (messageType == ServerMessageType::Hello) {
                auto len = socket.read_some(boost::asio::buffer(buf), error);
                if (error == boost::asio::error::eof)
                    break; // Connection closed cleanly by peer.
                else if (error) {
                    throw boost::system::system_error(error); // Some other error.
                }
                for (int i = 0; i < len; ++i) {
                    std::cout << buf[i] << ' ';
                }
            }
            else {
                //auto hello = HelloMessage(socket);
                //hello.print();
            }

            std::cout << '\n'; */
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
