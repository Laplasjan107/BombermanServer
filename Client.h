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
#include "Direction.h"
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
        boost::array<char, GUIBufferSize> GUIBuffer;
        std::string playerName;
        std::unique_ptr<boost::asio::io_context> context;
        std::unique_ptr<tcp::socket> serverSocket;
        std::unique_ptr<udp::socket> guiSocketWriter;
        udp::endpoint guiWriteEndpoint;
        std::unique_ptr<GameStatus> game;

        void handlePlaceBomb() {
            char place_bomb[1] = {1};
            boost::asio::write(*serverSocket, boost::asio::buffer(place_bomb, 1));
        }

        void handlePlaceBlock() {
            char place_block[1] = {2};
            boost::asio::write(*serverSocket, boost::asio::buffer(place_block, 1));
        }

        void handleMove() {
            char move[2] = {3, 0};
            move[1] = GUIBuffer[1];
            boost::asio::write(*serverSocket, boost::asio::buffer(move, 2));
        }

        [[noreturn]] void guiConnection() {
            udp::endpoint remote_endpoint;
            while (true) {
                auto messageSize = guiSocketWriter->receive_from(boost::asio::buffer(GUIBuffer), remote_endpoint);

                switch (GUIBuffer[0]) {
                    case (static_cast<uint8_t>(InputMessageType::PlaceBomb)):
                        if (messageSize == 1) {
                            if (game->isRunning()) {
                                handlePlaceBomb();
                            }
                            else {
                                sendJoinToServer();
                            }
                        }
                        break;
                    case (static_cast<uint8_t>(InputMessageType::PlaceBlock)):
                        if (messageSize == 1) {
                            if (game->isRunning()) {
                                handlePlaceBlock();
                            }
                            else {
                                sendJoinToServer();
                            }
                        }
                        break;
                    case (static_cast<uint8_t>(InputMessageType::Move)):
                        if (messageSize == 2 && GUIBuffer < 4) {
                            if (game->isRunning()) {
                                handleMove();
                            }
                            else {
                                sendJoinToServer();
                            }
                        }
                }
            }
        }

        void sendJoinToServer() {
            boost::array<uint8_t, 2> nameHeader{0, (uint8_t) playerName.length()};
            boost::asio::write(*serverSocket, boost::asio::buffer(nameHeader));
            boost::asio::write(*serverSocket, boost::asio::buffer(playerName));
        }

        void sendGameToGUI() {
            game->writeToUDP();
            UDPMessage::sendAndClear(*guiSocketWriter, guiWriteEndpoint);
        }


        void handleHelloMessage() {
            auto hello = HelloMessage(*serverSocket);
            game->initializeGame(hello);
        }

        void handleAcceptedPlayerMessage() {
            auto accepted = AcceptedPlayerMessage(*serverSocket);
            game->newPlayer(accepted);
        }

        void handleGameStartedMessage() {
            auto started = GameStartedMessage(*serverSocket);
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
            player_id_t playerId;
            read_number_inplace(*serverSocket, playerId);
            Position playerPosition{*serverSocket};
            game->movePlayer(playerId, playerPosition);
        }

        void handleBlockPlacedEvent() {
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
            game_length_t turnNumber;
            read_number_inplace(*serverSocket, turnNumber);

            game->newTurn(turnNumber);

            list_size_t eventsNumber;
            read_number_inplace(*serverSocket, eventsNumber);

            for (list_size_t i = 0; i < eventsNumber; ++i) {
                handleEvent();
            }
        }

        void handleGameEndedMessage() {
            GameEndedMessage gameEnded{*serverSocket};
            game->endGame(gameEnded);
        }

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
            boost::asio::ip::tcp::no_delay option(true);
            serverSocket->set_option(option);
        }

        void makeGUIConnection(ClientOptions &options) {
            guiSocketWriter = std::make_unique<udp::socket>(*context, udp::endpoint(udp::v6(), options.port));
            udp::resolver res(*context);
            udp::resolver::iterator iter = res.resolve(options.guiIP, options.guiPort);
            guiWriteEndpoint = *iter;
        }

    public:
        Client(ClientOptions &options) {
            game = std::make_unique<GameStatus>();
            context = std::make_unique<boost::asio::io_context>();

            makeServerConnection(options);
            makeGUIConnection(options);
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
