#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <unordered_set>
#include <set>
#include <map>
#include <queue>
#include "types.h"
#include "Player.h"
#include "messages/client-gui/UDPMessage.h"
#include "messages/server-client/ClientMessageType.h"
#include "Bomb.h"

#include "IServer.h"
#include "ServerOptions.h"
#include "Game.h"

namespace bomberman {
    using boost::asio::ip::tcp;

    struct ISession {
        virtual void sendMessage(std::vector<uint8_t> acceptedPlayer) = 0;

        virtual ~ISession() {};
    };

    class Session : public ISession, public std::enable_shared_from_this<Session> {
        std::unique_ptr<std::queue<std::vector<uint8_t>>> _messages;

        std::shared_ptr<IServer> _server;
    public:
        Session(tcp::socket socket, std::shared_ptr<Game> game, std::shared_ptr<IServer> server)
                : _server(std::move(server)),
                  _game(std::move(game)),
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
                                std::cerr << "[debug] wrote message, size of q = " << _messages->size() << '\n';
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
            std::cerr << "[debug] Hello bytes:\n";
            for (size_t i = 0; i < hello.size(); ++i)
                std::cerr << (int) ((uint8_t *) hello.data())[i] << ' ';
            std::cerr << "\n";

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
                               std::cerr << "[debug] Got message " << (int) _buffer[0] << "\n";
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
                                       _game->disconnectPlayer(sessionId);
                               }
                           } else {
                               _disconnect = true;
                               _game->disconnectPlayer(sessionId);
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
                               std::cerr << "[debug] Got name length = " << (int) _buffer[0] << '\n';
                               string_length_t nameLength = _buffer[0];
                               doReadJoinName(nameLength);
                           } else {
                               _game->disconnectPlayer(sessionId);
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
                               string address = socket_.remote_endpoint().address().to_string();
                               if (socket_.remote_endpoint().address().is_v6()) {
                                   address = "[" + address + "]";
                               }
                               address = address + ":" + std::to_string(socket_.remote_endpoint().port());
                               _game->joinPlayer(sessionId, name, address);
                               doReadHeader();
                           } else {
                               _game->disconnectPlayer(sessionId);
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
                               std::cerr << "[debug] Got move\n";
                               if (direction < 4) {
                                   _game->movePlayer(sessionId, direction);
                                   doReadHeader();
                               } else {
                                   _game->disconnectPlayer(sessionId);
                                   _disconnect = true;
                                   return;
                               }
                           } else {
                               _game->disconnectPlayer(sessionId);
                               _disconnect = true;
                           }
                       });
        }

    public:
        std::shared_ptr<Game> _game;
        tcp::socket socket_;
        static player_id_t nextId;
        player_id_t sessionId;
        bool _disconnect = false;

        static const constexpr size_t _bufferSize = 128;
        uint8_t _buffer[_bufferSize];
    };

    player_id_t  Session::nextId = 0;


    class Server : public IServer, public std::enable_shared_from_this<Server> {
        boost::asio::deadline_timer _timer;
        GameOptions _options;

    public:
        void sendToAll(std::vector<uint8_t> acceptedPlayer) override {
            std::cerr << "[debug] NOS = " << activePlayers.size() << " after ";
            clearConnections();
            std::cerr << activePlayers.size() << "\n";
            for (const auto &player: activePlayers) {
                std::cerr << "sending to: " << (int) player->sessionId << '\n';
                player->sendMessage(acceptedPlayer);
            }
        }

        void sendTurn(std::vector<uint8_t> turnHeader, turn_message events) override {
            sendToAll(turnHeader);
            for (auto &event: events) {
                sendToAll(event);
            }
        }


        Server(boost::asio::io_context &io_context, GameOptions options) :
                acceptor_(io_context, tcp::endpoint(tcp::v6(), options.port)),
                _turnTimer(options.turnTimer),
                _timer(io_context, boost::posix_time::milliseconds(options.turnTimer)),
                _options(options) {}

        Server(boost::asio::io_context &io_context, short port, const std::shared_ptr<Game> &game)
                : acceptor_(io_context, tcp::endpoint(tcp::v6(), port)),
                  _turnTimer(game->_gameOptions.turnTimer),
                  _timer(io_context, boost::posix_time::milliseconds(game->_gameOptions.turnTimer)),
                  _game(game) {}

        void startGame() {
            _game = std::make_shared<Game>(_options, shared_from_this());
            do_accept();
            do_iteration();
        }

    private:
        void do_accept() {
            acceptor_.async_accept(
                    [this](boost::system::error_code ec, tcp::socket socket) {
                        if (!ec) {
                            std::cerr << "[debug] Someone connected\n";
                            std::shared_ptr<Server> self = shared_from_this();
                            std::shared_ptr<IServer> ISelf = std::dynamic_pointer_cast<IServer>(self);
                            auto player = std::make_shared<Session>(std::move(socket),
                                                                    std::shared_ptr(_game),
                                                                    ISelf);
                            player->start();
                            activePlayers.insert(std::shared_ptr(player));
                        }

                        do_accept();
                    });
        }

        void do_iteration() {
            clearConnections();

            if (_game->isRunning()) {
                _game->newTurn();
            }

            std::cerr << "TURN TIMER = " << _turnTimer << "\n";

            _timer.expires_from_now(boost::posix_time::milliseconds(_turnTimer));
            _timer.async_wait(
                    [this](boost::system::error_code ec) {
                        if (!ec)
                            this->do_iteration();
                    }
            );
        }

        void clearConnections() {
            for (auto iterator = activePlayers.begin(); iterator != activePlayers.end();) {
                if ((*iterator)->_disconnect) {
                    std::cerr << "[debug] Disconnected " << (*iterator)->sessionId << "\n";
                    auto toErase = iterator;
                    ++iterator;
                    activePlayers.erase(toErase);
                } else {
                    ++iterator;
                }
            }
        }

        std::unordered_set<std::shared_ptr<Session>> activePlayers;
        tcp::acceptor acceptor_;
        std::shared_ptr<Game> _game;
        std::vector<uint8_t> _turnMessage;
        uint64_t _turnTimer;
    };
}

int main(int argc, char *argv[]) {
    using namespace bomberman;

    try {
        GameOptions options {argc, argv};
        boost::asio::io_context io_context;
        std::shared_ptr<Server> server = std::make_shared<Server>(io_context, options);
        server->startGame();

        io_context.run();
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}