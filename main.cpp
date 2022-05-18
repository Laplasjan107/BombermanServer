#include <iostream>
#include <string>
#include <exception>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using boost::asio::ip::tcp;

struct ClientOptions {
    std::string clientName;
    uint16_t port = 0;
    std::string serverAddress;
    std::string displayAddress;

    struct HelpException : public std::invalid_argument {
        explicit HelpException (const std::string &description) : invalid_argument(description) { }

        [[nodiscard]] const char *what () const noexcept override {
            return "Help message requested";
        }
    };

    ClientOptions(int argumentsCount, char *argumentsTable[]) {
        po::options_description description("Options parser");
        description.add_options()
                ("display-address,d", po::value<std::string>(), "Port number")
                ("help,h", "Display this help message")
                ("player-name,n", "Display this help message")
                ("port,p", po::value<uint16_t>(), "Port number")
                ("server-address,s", po::value<std::string>(), "Server address");

        po::variables_map programVariables;
        po::store(po::command_line_parser(argumentsCount, argumentsTable).options(description).run(), programVariables);
        po::notify(programVariables);

        if (programVariables.count("help"))
            throw HelpException("Asked for help message");

        if (programVariables.count("port")) {
            std::cerr << "Port number " << programVariables["port"].as<uint16_t>() << std::endl;
        }
    }
};

int main(int argc, char *argv[]) {
    try {
        ClientOptions client {argc, argv};
    }
    catch (ClientOptions::HelpException &exception) {
        std::cout << "This is Bomberman game client.\n"
                     "Flags:\n"
                     "    -h, --help\n"
                     "    -d, --display-address (Required)\n"
                     "    -n, --player-name (Required)\n"
                     "    -p, --port (Required)\n"
                     "    -s, --server-address (Required)\n";
    }

    return 0;
}
