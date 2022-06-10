#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <set>
#include "ServerOptions.h"
#include "Server.h"

static const constexpr char helpMessage[] =
        "This is Bomberman game server.\n"
        "Flags:\n"
        "    -b, --bomb-timer <u16> (required)\n"
        "    -c, --players-count <u8> (required)\n"
        "    -d, --turn-duration <u64> in ms (required)\n"
        "    -e, --explosion-radius <u16> (required)\n"
        "    -h, --help\n"
        "    -k, --initial-blocks <u16> (required)\n"
        "    -l, --game-length <u16> (required)\n"
        "    -n, --server-name <String> (required)\n"
        "    -p, --port <u16> (required)\n"
        "    -s, --seed <u32>\n"
        "    -x, --size-x <u16> (required)\n"
        "    -y, --size-y <u16> (required)\n";


int main(int argc, char *argv[]) {
    using namespace bomberman;
    using namespace std;

    try {
        GameOptions options{argc, argv};
        boost::asio::io_context io_context;
        shared_ptr<Server> server = make_shared<Server>(io_context, options);
        server->startGame();

        io_context.run();
    }
    catch (GameOptions::HelpException &e) {
        cout << helpMessage;
        return 0;
    }
    catch (std::exception &e) {
        cerr << "Server stopped: " << e.what() << "\n";
        return 1;
    }
}