//
// Created by Piotr Kamiński on 10/06/2022.
//

#ifndef ROBOTS_CLIENT_SESSION_H
#define ROBOTS_CLIENT_SESSION_H

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <unordered_set>
#include <set>
#include <queue>
#include "types.h"
#include "messages/server-client/ClientMessageType.h"

#include "IServer.h"
#include "ServerOptions.h"
#include "Game.h"

namespace bomberman {
    using boost::asio::ip::tcp;

    struct ISession {
        virtual void sendMessage(std::vector<uint8_t> acceptedPlayer) = 0;

        virtual ~ISession() = default;
    };

    class Session : public ISession, public std::enable_shared_from_this<Session> {
        std::unique_ptr<std::queue<std::vector<uint8_t>>> _messages;
    public:
        Session(tcp::socket socket, std::shared_ptr<Game> game)
                : _game(std::move(game)),
                  socket_(std::move(socket)),
                  sessionId(nextId++) {
            _messages = std::make_unique<std::queue<std::vector<uint8_t>>>();
        }

        void sendMessage(std::vector<uint8_t> message) override {
            using namespace boost::asio;
            auto self = shared_from_this();
            if (_messages->empty()) {
                async_write(socket_, buffer(message),
                            [this, self](boost::system::error_code ec, std::size_t) mutable {
                                if (!ec && !_disconnect) {
                                    if (!_messages->empty()) {
                                        auto next = _messages->front();
                                        _messages->pop();
                                        sendMessage(next);
                                    }
                                } else {
                                    _disconnect = true;
                                }
                            });
            } else {
                _messages->push(std::move(message));
            }
        }

        void start() {
            doReadHeader();

            doSendHello();
            if (_game->isRunning()) {
                doSendRunningMessage();
            } else {
                doSendAllAccepted();
            }
        }

        void doSendHello() {
            using namespace boost::asio;

            auto self = shared_from_this();
            auto hello = _game->helloMessage();
            sendMessage(_game->helloMessage());
        }

        void doSendRunningMessage() {
            using namespace boost::asio;
            auto self = shared_from_this();

            sendMessage(_game->_gameStarted);
            for (auto &turnFragment: _game->_allTurns)
                sendMessage(turnFragment);
        }

        void doSendAllAccepted() {
            using namespace boost::asio;
            auto self = shared_from_this();
            sendMessage(_game->_allAcceptedPlayers);
        }

    private:
        void doReadHeader() {
            using namespace boost::asio;

            auto self(shared_from_this());
            async_read(socket_,
                       buffer(_buffer, 1),
                       [this, self](boost::system::error_code ec, std::size_t) {
                           if (!ec && !_disconnect) {
                               uint8_t type = _buffer[0];
                               switch (type) {
                                   case static_cast<uint8_t>(ClientMessageType::Join):
                                       doReadJoinLength();
                                       break;
                                   case static_cast<uint8_t>(ClientMessageType::PlaceBomb):
                                       _game->placeBomb(sessionId);
                                       doReadHeader();
                                       break;
                                   case static_cast<uint8_t>(ClientMessageType::PlaceBlock):
                                       _game->placeBlock(sessionId);
                                       doReadHeader();
                                       break;
                                   case static_cast<uint8_t>(ClientMessageType::Move):
                                       doReadMove();
                                       break;
                                   default:
                                       _disconnect = true;
                               }
                           } else {
                               _disconnect = true;
                           }
                       });
        }

        void doReadJoinLength() {
            using namespace boost::asio;

            auto self(shared_from_this());
            async_read(socket_,
                       buffer(_buffer, sizeof(string_length_t)),
                       [this, self](boost::system::error_code ec, std::size_t) {
                           if (!ec && !_disconnect) {
                               string_length_t nameLength = _buffer[0];
                               doReadJoinName(nameLength);
                           } else {
                               _disconnect = true;
                           }
                       });
        }

        void doReadJoinName(string_length_t stringLength) {
            using namespace boost::asio;

            auto self(shared_from_this());
            async_read(socket_,
                       buffer(_buffer, stringLength),
                       [this, self](boost::system::error_code ec, std::size_t nameLength) {
                           if (!ec && !_disconnect) {
                               _buffer[nameLength] = '\0';
                               string name((char *) _buffer);
                               auto address = boost::lexical_cast<std::string>(socket_.remote_endpoint());
                               _game->joinPlayer(sessionId, name, address);
                               doReadHeader();
                           } else {
                               _disconnect = true;
                           }
                       });
        }

        void doReadMove() {
            using namespace boost::asio;

            auto self(shared_from_this());
            async_read(socket_,
                       buffer(_buffer, 1),
                       [this, self](boost::system::error_code ec, std::size_t) {
                           if (!ec && !_disconnect) {
                               uint8_t direction = _buffer[0];
                               if (direction < 4) {
                                   _game->movePlayer(sessionId, direction);
                                   doReadHeader();
                               } else {
                                   _disconnect = true;
                                   return;
                               }
                           } else {
                               _disconnect = true;
                           }
                       });
        }

    public:
        std::shared_ptr<Game> _game;
        tcp::socket socket_;
        static int nextId;
        int sessionId;
        bool _disconnect = false;

        static const constexpr size_t _bufferSize = 128;
        uint8_t _buffer[_bufferSize]{};
    };

    int Session::nextId = 0;


}

#endif //ROBOTS_CLIENT_SESSION_H
