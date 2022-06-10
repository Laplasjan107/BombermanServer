//
// Created by Piotr Kamiński on 10/06/2022.
//

#ifndef ROBOTS_CLIENT_SERVER_H
#define ROBOTS_CLIENT_SERVER_H

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <unordered_set>
#include <set>
#include <queue>
#include "types.h"

#include "IServer.h"
#include "ServerOptions.h"
#include "Game.h"
#include "Session.h"

namespace bomberman {
    using boost::asio::ip::tcp;

    class Server : public IServer, public std::enable_shared_from_this<Server> {
        boost::asio::deadline_timer _timer;
        ServerOptions _options;

    public:
        void sendToAll(std::vector<uint8_t> acceptedPlayer) override {
            clearConnections();
            for (const auto &player: activePlayers) {
                player->sendMessage(acceptedPlayer);
            }
        }

        void sendTurn(std::vector<uint8_t> turnHeader, turn_message events) override {
            sendToAll(turnHeader);
            for (auto &event: events) {
                sendToAll(event);
            }
        }


        Server(boost::asio::io_context &io_context, const ServerOptions &options) :
                _timer(io_context, boost::posix_time::milliseconds(options.turnDuration)),
                _options(options),
                acceptor_(io_context, tcp::endpoint(tcp::v6(), options.port)),
                _turnTimer(options.turnDuration) {}

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
                            auto player = std::make_shared<Session>(std::move(socket),
                                                                    std::shared_ptr(_game));
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
        uint64_t _turnTimer;
    };
}
#endif //ROBOTS_CLIENT_SERVER_H
