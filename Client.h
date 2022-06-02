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

#include "ClientOptions.h"
#include "MapSettings.h"
#include "UnexpectedMessageException.h"
#include "messages/messages.h"
#include "GameStatus.h"
#include "event/events.h"
#include "Client.h"

namespace bomberman {
    using boost::asio::ip::tcp;
    using boost::asio::ip::udp;

    class Client {
        static const constexpr size_t GUIBufferSize = 10;
        static const constexpr size_t maxDirection = 3;
        static const constexpr size_t moveDirectionPosition = 1;
        static const constexpr size_t GUIPlaceBombSize = 1;
        static const constexpr size_t GUIPlaceBlockSize = 1;
        static const constexpr size_t GUIMoveSize = 2;

        boost::array<uint8_t, GUIBufferSize> GUIBuffer;

        std::string playerName;
        std::unique_ptr<boost::asio::io_context> context;
        std::unique_ptr<tcp::socket> serverSocket;
        std::unique_ptr<udp::socket> GUISocket;
        udp::endpoint guiWriteEndpoint;
        std::unique_ptr<GameStatus> game;

        void sendJoinToServer() {
            static const constexpr size_t joinHeaderSize = 2;
            boost::array<uint8_t, joinHeaderSize> nameHeader{static_cast<uint8_t>(ClientMessageType::Join),
                                                             (uint8_t) playerName.length()};
            boost::asio::write(*serverSocket, boost::asio::buffer(nameHeader));
            boost::asio::write(*serverSocket, boost::asio::buffer(playerName));
        }

        void handlePlaceBomb() {
            static const constexpr size_t placeBombSize = 1;
            uint8_t place_bomb[placeBombSize] = {static_cast<uint8_t>(ClientMessageType::PlaceBomb)};
            boost::asio::write(*serverSocket, boost::asio::buffer(place_bomb, placeBombSize));
        }

        void handlePlaceBlock() {
            static const constexpr size_t placeBlockSize = 1;
            uint8_t place_block[placeBlockSize] = {static_cast<uint8_t>(ClientMessageType::PlaceBlock)};
            boost::asio::write(*serverSocket, boost::asio::buffer(place_block, placeBlockSize));
        }

        void handleMove() {
            static const constexpr size_t moveSize = 2;
            uint8_t move[moveSize] = {static_cast<uint8_t>(ClientMessageType::Move), 0};
            move[moveDirectionPosition] = GUIBuffer[moveDirectionPosition];
            boost::asio::write(*serverSocket, boost::asio::buffer(move, moveSize));
        }

        [[noreturn]] void guiConnection() {
            udp::endpoint remote_endpoint;
            while (true) {
                auto messageSize = GUISocket->receive_from(boost::asio::buffer(GUIBuffer), remote_endpoint);

                switch (GUIBuffer[0]) {
                    case (static_cast<uint8_t>(InputMessageType::PlaceBomb)):
                        if (messageSize == GUIPlaceBombSize) {
                            if (game->isRunning()) {
                                handlePlaceBomb();
                            } else {
                                sendJoinToServer();
                            }
                        }
                        break;
                    case (static_cast<uint8_t>(InputMessageType::PlaceBlock)):
                        if (messageSize == GUIPlaceBlockSize) {
                            if (game->isRunning()) {
                                handlePlaceBlock();
                            } else {
                                sendJoinToServer();
                            }
                        }
                        break;
                    case (static_cast<uint8_t>(InputMessageType::Move)):
                        if (messageSize == GUIMoveSize && GUIBuffer[moveDirectionPosition] <= maxDirection) {
                            if (game->isRunning()) {
                                handleMove();
                            } else {
                                sendJoinToServer();
                            }
                        }
                }
            }
        }

        void sendGameToGUI() {
            game->writeToUDP();
            UDPMessage::sendAndClear(*GUISocket, guiWriteEndpoint);
        }


        void handleHelloMessage() {
            auto hello = HelloMessage(*serverSocket);
            std::cerr << "[debug] Hello message received\n";
            game->initializeGame(hello);
        }

        void handleAcceptedPlayerMessage() {
            auto accepted = AcceptedPlayerMessage(*serverSocket);
            std::cerr << "[debug] Accepted player received: "
                << (int) accepted.playerId << " " << accepted.player.playerName << " "
                << accepted.player.playerAddress << "\n";
            game->newPlayer(accepted);
        }

        void handleGameStartedMessage() {
            auto started = GameStartedMessage(*serverSocket);
            std::cerr << "[debug] Game started received: ";
            for (auto e: started.activePlayers) {
                std::cerr << (int) e.first << " " << e.second.playerName << " " << e.second.playerAddress << "\n";
            }
            game->startGame(started);
        }

        void handleBombPlacedEvent() {
            BombPlacedEvent bombPlaced{*serverSocket};
            game->placeBomb(bombPlaced);
        }

        void handleBombExplodedEvent() {
            BombExplodedEvent bombExplodedEvent{*serverSocket};
            game->explodeBomb(bombExplodedEvent);
        }

        void handlePlayerMovedEvent() {
            std::cerr << "[debug] Got player moved\n";
            player_id_t playerId;
            read_number_inplace(*serverSocket, playerId);
            Position playerPosition{*serverSocket};
            game->movePlayer(playerId, playerPosition);
            std::cerr << "[debug] Player id = " << (int) playerId << ", position = " << playerPosition << "\n";
        }

        void handleBlockPlacedEvent() {
            Position blockPosition{*serverSocket};
            game->placeBlock(blockPosition);
        }

        void handleEvent() {
            std::cerr << "[debug] Got event\n";
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
            std::cerr << "[debug] Turn message received.\n";
            game_length_t turnNumber;
            read_number_inplace(*serverSocket, turnNumber);
            std::cerr << "[debug] Turn number: " << turnNumber << "\n";

            game->newTurn(turnNumber);

            list_size_t eventsNumber;
            read_number_inplace(*serverSocket, eventsNumber);
            std::cerr << "[debug] Turn events: " << eventsNumber << "\n";

            for (list_size_t i = 0; i < eventsNumber; ++i) {
                handleEvent();
            }
        }

        void handleGameEndedMessage() {
            GameEndedMessage gameEnded{*serverSocket};
            game->endGame(gameEnded);
        }

        // Main server connection handler.
        void serverConnection() {
            while (true) {
                message_header_t messageType;
                boost::asio::read(*serverSocket, boost::asio::buffer(&messageType, sizeof(messageType)));
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
            tcp::no_delay option(true);
            serverSocket->set_option(option);
        }

        void makeGUIConnection(ClientOptions &options) {
            GUISocket = std::make_unique<udp::socket>(*context, udp::endpoint(udp::v6(), options.port));
            udp::resolver resolver(*context);
            udp::resolver::iterator iterator = resolver.resolve(options.guiIP, options.guiPort);
            guiWriteEndpoint = *iterator;
        }

    public:
        Client(ClientOptions &options) {
            game = std::make_unique<GameStatus>();
            context = std::make_unique<boost::asio::io_context>();

            makeServerConnection(options);
            makeGUIConnection(options);
            playerName = options.playerName;
        }

        ~Client() {
            GUISocket->close();
            serverSocket->close();
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
