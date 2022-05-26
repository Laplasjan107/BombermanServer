//
// Created by Piotr Kamiński on 19/05/2022.
//

#ifndef BOMBERMANSERVER_CLIENT_H
#define BOMBERMANSERVER_CLIENT_H

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
#include "Client.h"

//namespace po = boost::program_options;
namespace bomberman {
    using boost::asio::ip::tcp;
    using boost::asio::ip::udp;

    class Client {
        std::string playerName;

        std::unique_ptr<boost::asio::io_context> context;
        std::unique_ptr<tcp::socket> serverSocket;
        std::unique_ptr<udp::socket> guiSocketWriter;
        udp::endpoint guiWriteEndpoint;
//        std::unique_ptr<udp::socket> guiSocketReceiver;
        std::unique_ptr<GameStatus> game;

        [[noreturn]] void guiConnection() {
            boost::array<char, 10> recv_buf{};
            udp::endpoint remote_endpoint;

            while (true) {
                auto r = guiSocketWriter->receive_from(boost::asio::buffer(recv_buf), remote_endpoint);
                std::cout << "FROM GUI\n";
                for (int i = 0; i < r; ++i) {
                    std::cout << (int) recv_buf[i] << ' ';
                }
                std::cout << std::endl;

                bool valid = false;

                if (recv_buf[0] == 0 && r == 1) {
			if (game->isRunning()) {
                    char place_bomb[1] = {1};
			std::cerr << "TO SERVER " << 1 << std::endl;
                    boost::asio::write(*serverSocket, boost::asio::buffer(place_bomb, 1));
			}
                    valid = true;
                }
                else if (recv_buf[0] == 1 && r == 1) {
			if (game->isRunning()) {
                    char place_block[1] = {2};
			std::cerr << "TO SERVER " << 2 << std::endl;
                    boost::asio::write(*serverSocket, boost::asio::buffer(place_block, 1));
			}
                    valid = true;
                }
                else if (recv_buf[0] == 2 && r == 2) {
			if (game->isRunning()) {
                    char move[2] = {3, 0};
                    move[1] = recv_buf[1];
			std::cerr << "TO SERVER " << 3 << ' ' << recv_buf[1] << std::endl;
                    boost::asio::write(*serverSocket, boost::asio::buffer(move, 2));
			}
                    valid = true;
                }

                if (valid && !game->isRunning()) {
                    //boost::array<char, 8> m{0, 6, 'r', 't', 'o', 'i', 'p', 'K'};
			std::cerr << "TO SERVER " << 0 << ' ' << (unsigned int) playerName.length() << ' ';
                    boost::array<uint8_t, 2> m {0, (uint8_t) playerName.length()};
                    boost::asio::write(*serverSocket, boost::asio::buffer(m));
			for (auto c : playerName)
				std::cerr << 0 + c << ' ';
			std::cerr << std::endl;
                    boost::asio::write(*serverSocket, boost::asio::buffer(playerName));
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
            std::cerr << "BOMB PLACED ";
            BombPlacedEvent bombPlaced {*serverSocket};
		std::cerr << '(' << bombPlaced.position.positionX << ", " << bombPlaced.position.positionY << ")\n";
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
            game->movePlayer(playerId, playerPosition);
        }

        void handleBlockPlacedEvent() {
            std::cerr << "[debug] [Event] Block placed. ";
            Position blockPosition{*serverSocket};
		std::cerr << blockPosition << '\n';
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
            std::cerr << "[debug] [Message] Turn message received.\n";

            game_length_t turnNumber;
            read_number_inplace(*serverSocket, turnNumber);

            game->newTurn(turnNumber);

            list_size_t eventsNumber;
            read_number_inplace(*serverSocket, eventsNumber);

            std::cerr << "turn number = " << turnNumber << "EvenentsNumber = " << eventsNumber << std::endl;
            for (list_size_t i = 0; i < eventsNumber; ++i) {
                handleEvent();
            }
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
                bool refreshGUI = true;

                switch (messageType) {
                    case ServerMessageType::Hello:
                        handleHelloMessage();
                        break;
                    case ServerMessageType::AcceptedPlayer:
                        handleAcceptedPlayerMessage();
                        break;
                    case ServerMessageType::GameStarted:
                        handleGameStartedMessage();
                        refreshGUI = false;
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

                if (refreshGUI)
                    sendGameToGUI();
            }
        }

        void makeServerConnection(ClientOptions &options) {
            tcp::resolver resolver(*context);
            tcp::resolver::results_type endpoints = resolver.resolve(options.serverIP, options.serverPort);
            serverSocket = std::make_unique<tcp::socket>(*context);
            boost::asio::connect(*serverSocket, endpoints);
		boost::asio::ip::tcp::no_delay option(true);
	serverSocket->set_option(option);
        }

        void bindGuiWriter(ClientOptions &options) {
            guiSocketWriter = std::make_unique<udp::socket>(*context, udp::endpoint(udp::v6(), options.port));
            udp::resolver res(*context);
//            udp::resolver::query query(udp::v6(), options.guiIP, options.guiPort);
//            udp::resolver::iterator iter = res.resolve(query);
		udp::resolver::iterator iter = res.resolve(udp::v6(), options.guiIP, options.guiPort);
            guiWriteEndpoint = *iter;
        }

        void bindGuiListener(ClientOptions &options) {
            //guiSocketReceiver = std::make_unique<udp::socket>(*context, udp::endpoint(udp::v6(), options.port));
        }

    public:
        Client(ClientOptions &options) {
            game = std::make_unique<GameStatus>();
            context = std::make_unique<boost::asio::io_context>();

            makeServerConnection(options);
            bindGuiListener(options);
            bindGuiWriter(options);
            playerName = options.playerName;
        }

        void run() {
            std::thread gui_thread(&Client::guiConnection, this);
            std::thread server_thread(&Client::serverConnection, this);

            gui_thread.join();
            server_thread.join();
        }
    };
}

#endif //BOMBERMANSERVER_CLIENT_H
