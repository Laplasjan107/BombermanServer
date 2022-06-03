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
#include "Bomb.h"

namespace bomberman {
    using boost::asio::ip::tcp;

    struct IServer {
        virtual void sendToAll(std::vector<uint8_t> acceptedPlayer) = 0;

        virtual ~IServer() {};
    };

    struct ISession {
        virtual void sendMessage(std::vector<uint8_t> acceptedPlayer) = 0;

        virtual ~ISession() {};
    };

    struct GameOptions {
        std::string serverName;
        players_count_t playerCount;
        board_size_t sizeX;
        board_size_t sizeY;
        game_length_t gameLength;
        explosion_radius_t explosionRadius;
        bomb_timer_t bombTimer;
        uint64_t turnTimer;

        GameOptions() {
            serverName = "to_jest_serwer";
            playerCount = 2;
            sizeX = 15;
            sizeY = 15;
            gameLength = 500;
            explosionRadius = 3;
            bombTimer = 4;
            turnTimer = 200;
        }
    };

    struct Game {
        static size_t random() {
            static size_t last = 324;
            last *= 48271;
            last %= 2147483647;
            return last;
        }

        Position randomPosition() const {
            return {static_cast<board_size_t>(random() % _gameOptions.sizeX),
                    static_cast<board_size_t>(random() % _gameOptions.sizeY)};
        }

        std::vector<uint8_t> turns;
        static const constexpr Position versors[4] = {{0, 1},
                                                      {1, 0},
                                                      {0, (board_size_t) -1},
                                                      {(board_size_t) -1, 0},
        };


        explicit Game(GameOptions &gameOptions, std::shared_ptr<IServer> server) :
                _gameOptions(gameOptions),
                _isRunning(false),
                _server(std::move(server)) {
            _bombs = std::vector<std::vector<std::pair<Bomb, bomb_id_t>>>(gameOptions.bombTimer);
            generateHello();
        }

        void generateHello() {
            UDPMessage::clearBuffer();
            auto message = UDPMessage::getInstance();
            message << (uint8_t) 0
                    << _gameOptions.serverName
                    << _gameOptions.playerCount
                    << _gameOptions.sizeX
                    << _gameOptions.sizeY
                    << _gameOptions.gameLength
                    << _gameOptions.explosionRadius
                    << _gameOptions.bombTimer;
            _helloBuffer = UDPMessage::getBuffer();
        }

        bool isRunning() const {
            return _isRunning;
        }

        void disconnectPlayer(player_id_t playerId) {

        }

        void joinPlayer(player_id_t playerId, string playerName, string address) {
            std::cerr << "[debug] Got join player, id" << (int) playerId << " name "
                      << playerName << " address " << address << '\n';
            if (!isRunning() && !players.contains(playerId)) {
                static const uint8_t acceptedHeader = 1;
                Player newPlayer{std::move(playerName), std::move(address)};
                players.insert({playerId, newPlayer});

                UDPMessage::clearBuffer();
                UDPMessage::getInstance() << acceptedHeader
                                          << playerId
                                          << newPlayer.playerName
                                          << newPlayer.playerAddress;
                auto recentlyAcceptedPlayer = UDPMessage::getBuffer();
                //_allAcceptedPlayers.insert(_allAcceptedPlayers.end(), recentlyAcceptedPlayer.begin(),
                //                           recentlyAcceptedPlayer.end());

                _allAcceptedPlayers.insert(
                        _allAcceptedPlayers.end(),
                        recentlyAcceptedPlayer.begin(),
                        recentlyAcceptedPlayer.end());

                std::cerr << "[debug] new player buffer\n";
                for (auto e: newPlayer.toBuffer())
                    std::cerr << (int) e << ' ';
                std::cerr << "\n";

                std::cerr << "[debug] send accepted to all\n";
                for (auto e: recentlyAcceptedPlayer)
                    std::cerr << (int) e << ' ';
                std::cerr << "\n";

                _server->sendToAll(recentlyAcceptedPlayer);
                std::cerr << "[debug] Ready " << players.size() << '/' << (int) _gameOptions.playerCount << '\n';
                if (players.size() == _gameOptions.playerCount) {
                    startGame();
                }
            }
        }

        void startGame() {
            std::cerr << "[debug] Start game\n";
            _isRunning = true;
            UDPMessage::clearBuffer();
            UDPMessage::getInstance()
                    << (uint8_t) 2 // GameStarted
                    << players;

            _gameStarted = UDPMessage::getBuffer();
            std::cerr << "[debug] Game message: ";
            for (auto e: _gameStarted)
                std::cerr << (int) e << ' ';
            std::cerr << '\n';
            _server->sendToAll(_gameStarted);

            newTurn();
        }


        void placeBomb(player_id_t playerId) {
            clearLastMove(playerId);
            _newBomb[playerId] = Bomb(_playerPositions[playerId], _gameOptions.bombTimer);
        }

        void placeBlock(player_id_t playerId) {
            clearLastMove(playerId);
            _newBlock[playerId] = _playerPositions[playerId];
        }

        void movePlayer(player_id_t playerId, uint8_t direction) {
            clearLastMove(playerId);
            std::cerr << "[debug] Got move in direction: " << versors[direction] << "\n";
            Position newPosition = _playerPositions[playerId] + versors[direction];
            std::cerr << "[debug] New position: " << newPosition << "\n";
            if (isInside(newPosition)) {
                std::cerr << "[debug] Is inside\n";
                _newPlayerPosition[playerId] = newPosition;
            }
        }

        bool isInside(const Position &position) const {
            return position.positionX < _gameOptions.sizeX &&
                   position.positionY < _gameOptions.sizeY;
        }

        void newTurn() {
            std::cerr << "[debug] New turn " << turn << "\n";
            events = 0;
            size_t explodingId = turn % _gameOptions.bombTimer;
            UDPMessage::clearBuffer();
            UDPMessage::getInstance()
                    << (uint8_t) 3              // turn header
                    << turn
                    << events;                 // Placeholder, until the number of events is determined.
            for (auto &bomb: _bombs[explodingId]) {
                calculateBombOutcome(bomb);
            }

            _bombs[explodingId].clear();

            calculateMovesOutcome();
            if (turn == 0) {
                generateBlocks();
            }

            UDPMessage::loadAtIndex(sizeof(uint8_t) + sizeof(turn), events);

            auto turnMessage = UDPMessage::getBuffer();
            turns.insert(turns.end(), turnMessage.begin(), turnMessage.end());

            std::cerr << "[debug] Turn message: ";
            for (auto e: turnMessage)
                std::cerr << (int) e << ' ';
            std::cerr << '\n';

            doCleanUp();

            _server->sendToAll(turnMessage);
            if (turn == _gameOptions.gameLength) {
                endGame();
            }

            ++turn;
        }

        void doCleanUp() {
            for (const auto &dead: _playersDestroyer) {
                _alive.erase(dead);
                _playerPositions.erase(dead);
                ++_scores[dead];
            }

            for (const auto &block: _blocksDestroyed) {
                _blocks.erase(block);
            }

            clearMoves();
        }

        void calculateBombOutcome(std::pair<Bomb, bomb_id_t> &bomb) {
            ++events;
            std::unordered_set<player_id_t> playersDestroyed;
            std::unordered_set<Position> blocksDestroyed;
            for (const auto &versor: versors) {
                for (explosion_radius_t i = 0; i <= _gameOptions.explosionRadius; ++i) {
                    Position current = bomb.first.bombPosition + versor * i;
                    if (!isInside(current))
                        break;

                    for (auto &player: _playerPositions) {
                        if (player.second == current) {
                            playersDestroyed.insert(player.first);
                            _playersDestroyer.insert(player.first);
                        }
                    }

                    if (_blocks.contains(current)) {
                        blocksDestroyed.insert(current);
                        _blocks.insert(current);
                        break;
                    }
                }
            }
            UDPMessage::getInstance()   << (uint8_t) 1 // Bomb exploded
                                        << bomb.second
                                        << playersDestroyed
                                        << blocksDestroyed;
        }

        void generateBlocks() {

        }

        void calculateMovesOutcome() {
            std::cerr << "[debug] Calculate moves outcome: " << players.size() << "\n";
            for (const auto &player: players) {
                auto playerId = player.first;
                std::cerr << "[debug] Now player " << (int) playerId << "\n";
                if (_alive.contains(playerId)) {
                    if (_newPlayerPosition.contains(playerId)) {
                        _playerPositions[playerId] = _newPlayerPosition[playerId];
                        loadPlayerMovedEvent(playerId);
                    } else if (_newBlock.contains(playerId)) {
                        auto placed = _newBlock[playerId];
                        if (!_blocks.contains(placed)) {
                            _blocks.insert(placed);
                            loadPlacedBlockEvent(placed);
                        }
                    } else if (_newBomb.contains(playerId)) {
                        loadBombPlacedEvent(_newBomb[playerId]);
                    }
                } else {
                    _newPlayerPosition[playerId] = randomPosition();
                    std::cerr << "[debug] Random position for player " << (int) playerId << " "
                              << _newPlayerPosition[playerId] << '\n';
                    _playerPositions[playerId] = _newPlayerPosition[playerId];
                    loadPlayerMovedEvent(playerId);
                    _alive.insert(playerId);
                }
            }
        }

        void loadPlayerMovedEvent(player_id_t playerId) {
            ++events;
            UDPMessage::getInstance() << (uint8_t) 2 << playerId << _newPlayerPosition[playerId];
        }

        void loadPlacedBlockEvent(const Position &position) {
            ++events;
            UDPMessage::getInstance() << (uint8_t) 3 << position;
        }

        void loadBombPlacedEvent(Bomb &bomb) {
            ++events;
            UDPMessage::getInstance() << (uint8_t) 0 << nextBombId << bomb.bombPosition;
            _bombs[turn % _gameOptions.bombTimer].push_back({bomb, nextBombId});
            ++nextBombId;
        }


        const std::vector<uint8_t> &allTurns() {
            return turns;
        }

        const std::vector<uint8_t> &helloMessage() const {
            return _helloBuffer;
        }

        void clearMoves() {
            _newPlayerPosition.clear();
            _newBomb.clear();
            _newBlock.clear();
            _playersDestroyer.clear();
            _blocksDestroyed.clear();
        }

        void clearLastMove(player_id_t playerId) {
            _newPlayerPosition.erase(playerId);
            _newBomb.erase(playerId);
            _newBlock.erase(playerId);
        }

        void endGame() {
            UDPMessage::clearBuffer();
            UDPMessage::getInstance() << (uint8_t) 4 << _scores;
            _server->sendToAll(UDPMessage::getBuffer());
            _isRunning = false;
            players.clear();
            _playerPositions.clear();
            _blocks.clear();
            _scores.clear();
            _allAcceptedPlayers.clear();
            _gameStarted.clear();
            for (auto &bombBucket: _bombs)
                bombBucket.clear();
            _alive.clear();
            turns.clear();
            clearMoves();
            turn = 0;
        }

        GameOptions _gameOptions;
        bool _isRunning;
        players_t players;
        std::vector<uint8_t> _helloBuffer;
        std::vector<uint8_t> _allAcceptedPlayers;
        std::vector<uint8_t> _gameStarted;
        std::shared_ptr<IServer> _server;
        std::vector<std::vector<std::pair<Bomb, bomb_id_t>>> _bombs;
        uint16_t turn = 0;
        list_size_t events = 0;

        bomb_id_t nextBombId = 0;
        std::unordered_map<player_id_t, Position> _playerPositions;
        std::unordered_set<Position> _blocks;
        std::unordered_map<player_id_t, score_t> _scores;

        std::unordered_map<player_id_t, Position> _newPlayerPosition;
        std::unordered_map<player_id_t, Bomb> _newBomb;
        std::unordered_map<player_id_t, Position> _newBlock;
        std::unordered_set<player_id_t> _alive;

        std::unordered_set<player_id_t> _playersDestroyer;
        std::unordered_set<Position> _blocksDestroyed;
    };

    class Session : public ISession, public std::enable_shared_from_this<Session> {
        std::unique_ptr<std::queue<std::vector<uint8_t>>> _messages;
        bool _sending = false;

    public:
        Session(tcp::socket socket, std::shared_ptr<Game> game, std::shared_ptr<IServer> server)
                : _game(game),
                  socket_(std::move(socket)),
                  sessionId(nextId++) {
            _messages = std::make_unique<std::queue<std::vector<uint8_t>>>();
        }

        void sendMessage(std::vector<uint8_t> message) override {
            using namespace boost::asio;
            auto self = shared_from_this();
            if (_messages->empty()) {
                _sending = true;
                async_write(socket_, buffer(message),
                            [this, self](boost::system::error_code ec, std::size_t) mutable {
                                std::cerr << "[debug] wrote message, size of q = " << _messages->size() << '\n';
                                if (!ec && !_disconnect) {
                                    if (_messages->empty()) {
                                        _sending = false;
                                    } else {
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

            std::vector<uint8_t> toSend = _game->_gameStarted;
            toSend.insert(toSend.end(), _game->turns.begin(), _game->turns.end());
            sendMessage(toSend);
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
                                   case 0:
                                       doReadJoinLength();
                                       break;
                                   case 1:
                                       _game->placeBomb(sessionId);
                                       doReadHeader();
                                       break;
                                   case 2:
                                       _game->placeBlock(sessionId);
                                       doReadHeader();
                                       break;
                                   case 3:
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
                               /*
                               if (socket_.remote_endpoint().address().is_v6())
                                    std::cerr << "[debug] joined: " << socket_.remote_endpoint().address().to_v6().to_string() << "\n";
                               else
                                   std::cerr << "[debug] joined: " << socket_.remote_endpoint().address().to_v4().to_string() << "\n";
                               */
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
                       [this, self](boost::system::error_code ec, std::size_t nameLength) {
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


        Server(boost::asio::io_context &io_context, short port, GameOptions options) :
                acceptor_(io_context, tcp::endpoint(tcp::v6(), port)),
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
            } else {

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
                    std::cerr << "[debug] Disconnected " << (*iterator)->sessionId << "\n";
                    auto toErase = iterator;
                    ++iterator;
                    activePlayers.erase(toErase);
                } else {
                    ++iterator;
                }
            }
        }

        void sendTurnToAll() {
            std::cerr << "[debug] Send turn to " << activePlayers.size() << "\n";
            for (auto iterator = activePlayers.begin(); iterator != activePlayers.end(); ++iterator) {
                std::cerr << "[debug] Active player\n";
                (*iterator)->sendMessage(_turnMessage);
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
        GameOptions options;
        //std::shared_ptr<Game> game = std::make_shared<Game>(options);
        //std::shared_ptr<Server> serv = std::make_shared<Server>(io_context, 12345, std::shared_ptr(game));
        boost::asio::io_context io_context;
        std::shared_ptr<Server> server = std::make_shared<Server>(io_context, 12345, options);
        server->startGame();

        io_context.run();
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}