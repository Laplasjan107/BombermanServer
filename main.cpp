#include <iostream>
#include <string>
#include <exception>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <thread>
//#include <boost/program_options.hpp>

#include "ClientOptions.h"
#include "Direction.h"
#include "MapSettings.h"
#include "UnexpectedMessageException.h"
#include "messages/messages.h"
#include "GameStatus.h"
#include "event/events.h"

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

namespace bomberman {
    class Client {
        std::unique_ptr<boost::asio::io_context> context;
        std::unique_ptr<tcp::socket> serverSocket;
        std::unique_ptr<udp::socket> guiSocketWriter;
        udp::endpoint guiWriteEndpoint;
        std::unique_ptr<udp::socket> guiSocketReceiver;
        std::unique_ptr<GameStatus> game;

        [[noreturn]] void guiConnection() {
            boost::array<char, 10> recv_buf{};
            udp::endpoint remote_endpoint;

            while (true) {
                auto r = guiSocketReceiver->receive_from(boost::asio::buffer(recv_buf), remote_endpoint);
                std::cout << "FROM GUI\n";
                for (int i = 0; i < r; ++i) {
                    std::cout << (int) recv_buf[i] << ' ';
                }
                std::cout << std::endl;

                bool valid = false;

                if (recv_buf[0] == 0 && r == 1) {
                    char place_bomb[1] = {1};
                    boost::asio::write(*serverSocket, boost::asio::buffer(place_bomb, 1));
                    valid = true;
                }
                else if (recv_buf[0] == 1 && r == 1) {
                    char place_block[1] = {2};
                    boost::asio::write(*serverSocket, boost::asio::buffer(place_block, 1));
                    valid = true;
                }
                else if (recv_buf[0] == 2 && r == 2) {
                    char move[2] = {3, 0};
                    move[1] = recv_buf[1];
                    boost::asio::write(*serverSocket, boost::asio::buffer(move, 2));
                    valid = true;
                }

                std::cerr << "VALID = " << valid << " " << "RUNNING = " << game->running << '\n';

                if (valid && !game->running) {
                    boost::array<char, 6> m{0, 4, 'a', 'b', 'c', 'd'};
                    std::cerr << "WRITING NAME TO BUFFER\n";
                    boost::asio::write(*serverSocket, boost::asio::buffer(m));
                }
            }

        }

        void sendGameToGUI() {
            game->writeToUDP();
            UDPMessage::sendAndClear(*guiSocketWriter, guiWriteEndpoint);
        }


        void handleHelloMessage() {
            std::cerr << "Hello message\n";
            auto hello = HelloMessage(*serverSocket);
            hello.print();
            game->initializeGame(hello);
        }

        void handleAcceptedPlayerMessage() {
            std::cerr << "Accepted player\n";
            auto accepted = AcceptedPlayerMessage(*serverSocket);
            game->newPlayer(accepted);
        }

        void handleGameStartedMessage() {
            std::cerr << "Game started message\n";
            auto started = GameStartedMessage(*serverSocket);
            started.print();
            game->startGame(started);
        }

        void handleBombPlacedEvent() {
            std::cerr << "BOMB PLACED\n";
            BombPlacedEvent bombPlaced {*serverSocket};
            game->placeBomb(bombPlaced);
        }

        void handleBombExplodedEvent() {
            std::cerr << "[debug] [Event] Bomb exploded.\n";
            BombExplodedEvent bombExplodedEvent{*serverSocket};
            game->explodeBomb(bombExplodedEvent);
        }

        void handlePlayerMovedEvent() {
            std::cerr << "[debug] [Event] Player moved. ";
            player_id_t playerId;
            read_number_inplace(*serverSocket, playerId);
            Position playerPosition{*serverSocket};
            std::cerr << "Player id: " << (int) playerId << ", new position: " << playerPosition << '\n';
            game->positions[playerId] = playerPosition;
        }

        void handleBlockPlacedEvent() {
            std::cerr << "[debug] [Event] Block placed.\n";
            Position blockPosition{*serverSocket};
            game->placeBlock(blockPosition);
        }

        void handleEvent() {
            message_header_t eventHeader;
            read_number_inplace(*serverSocket, eventHeader);
            switch (eventHeader) {
                case EventType::BombPlaced:
                    handleBombPlacedEvent();
                    break;
                case EventType::BombExploded:
                    handleBombExplodedEvent();
                    break;
                case EventType::PlayerMoved:
                    handlePlayerMovedEvent();
                    break;
                case EventType::BlockPlaced:
                    handleBlockPlacedEvent();
                    break;
                default:
                    throw UnexpectedMessageException();
            }
        }

        void handleTurnMessage() {
            std::cerr << "TURN MESSAGE: ";
            game_length_t turnNumber;
            read_number_inplace(*serverSocket, turnNumber);

            list_size_t eventsNumber;
            read_number_inplace(*serverSocket, eventsNumber);
            game->turn = turnNumber;
            game->clearExplosions();
            std::cerr << "turn number = " << turnNumber << "EvenentsNumber = " << eventsNumber << std::endl;
            for (list_size_t i = 0; i < eventsNumber; ++i) {
                handleEvent();
            }
            std::cerr << "TURN MESSAGE HANDELED\n";
        }

        void handleGameEndedMessage() {
            std::cerr << "[debug] [Message] Game ended.\n";
            GameEndedMessage gameEnded{*serverSocket};
            game->endGame(gameEnded);
        }

        void serverConnection() {
            while (true) {
                message_header_t messageType;
                boost::asio::read(*serverSocket, boost::asio::buffer(&messageType, sizeof(messageType)));
                std::cerr << "New message " << (int) messageType << std::endl;

                switch (messageType) {
                    case ServerMessageType::Hello:
                        handleHelloMessage();
                        break;
                    case ServerMessageType::AcceptedPlayer:
                        handleAcceptedPlayerMessage();
                        break;
                    case ServerMessageType::GameStarted:
                        handleGameStartedMessage();
                        break;
                    case ServerMessageType::Turn:
                        handleTurnMessage();
                        break;
                    case ServerMessageType::GameEnded:
                        handleGameEndedMessage();
                        break;
                    default:
                        throw UnexpectedMessageException();
                }
                sendGameToGUI();
            }
        }

    public:
        Client(ClientOptions &options) {
            game = std::make_unique<GameStatus>();
            context = std::make_unique<boost::asio::io_context>();

            tcp::resolver resolver(*context);
            tcp::resolver::results_type endpoints = resolver.resolve("students.mimuw.edu.pl", "10015");
            serverSocket = std::make_unique<tcp::socket>(*context);
            boost::asio::connect(*serverSocket, endpoints);

            guiSocketWriter = std::make_unique<udp::socket>(*context, udp::endpoint(udp::v6(), 0));
            udp::resolver res(*context);
            udp::resolver::query query(udp::v6(), "localhost", "12345");
            udp::resolver::iterator iter = res.resolve(query);
            guiWriteEndpoint = *iter;

            guiSocketReceiver = std::make_unique<udp::socket>(*context, udp::endpoint(udp::v6(), 14008));
        }

        void run() {
            std::thread gui_thread(&Client::guiConnection, this);
            std::thread server_thread(&Client::serverConnection, this);

            gui_thread.join();
            server_thread.join();
        }
    };
}


int main(int argc, char *argv[]) {
    using namespace std;

    try {
        ClientOptions options {argc, argv};
        Client client {options};
        client.run();
    }
    catch (ClientOptions::HelpException &exception) {
        cout << helpMessage << endl;
    }
    catch (exception &exception) {
        cerr << exception.what() << endl;
    }

    return 0;
}
