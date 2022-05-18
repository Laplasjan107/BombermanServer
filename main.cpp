#include <iostream>
#include <string>
#include <exception>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

struct ClientOptions {
    std::string clientName;
    uint16_t port;
    std::string serverAddress;
    std::string displayAddress;

    struct HelpException : public std::invalid_argument {
        HelpException(const std::string &desctiption) : invalid_argument(desctiption) { }

        const char * what () const throw () {
            return "Display help message";
        }
    };

    ClientOptions(int argumentsCount, char *argumentsTable[]) {
        po::options_description description("Options parser");
        description.add_options()
                ("display-address,d", po::value<int>(), "Port number")
                ("help,h", "Display this help message")
                ("player-name,n", "Display this help message")
                ("port,p", po::value<int>(), "Port number")
                ("server-address,s", po::value<std::string>(), "Server address");

        po::variables_map programVariables;
        po::store(po::command_line_parser(argumentsCount, argumentsTable).options(description).run(), programVariables);
        po::notify(programVariables);

        if (programVariables.count("help"))
            throw HelpException("Asked for help message");

        if (programVariables.count("port")) {
            std::cerr << "Port number " << programVariables["port"].as<int>() << std::endl;
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
                     "    -d, --display-address <(nazwa hosta):(port) lub (IPv4):(port) lub (IPv6):(port)>\n"
                     "    -h, --help                                 Print help information\n"
                     "    -n, --player-name <String>\n"
                     "    -p, --port <u16>\n"
                     "    -s, --server-address <(nazwa hosta):(port) lub (IPv4):(port) lub (IPv6):(port)>\n";
    }


    std::cout << "Hello, World!" << std::endl;

    return 0;
}
