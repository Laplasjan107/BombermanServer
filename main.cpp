#include <iostream>
#include <exception>

#include "ClientOptions.h"
#include "Client.h"

static const constexpr char helpMessage[] = "This is Bomberman game client.\n"
                     "Flags:\n"
                     "    -h, --help\n"
                     "    -d, --display-address (Required)\n"
                     "    -n, --player-name (Required)\n"
                     "    -p, --port (Required)\n"
                     "    -s, --server-address (Required)\n";


int main(int argc, char *argv[]) {
    using namespace std;
    using namespace bomberman;

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
        return 1;
    }

    return 0;
}
