#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

struct ClientOptions {
    std::string clientName;
    uint16_t port;
    std::string serverAddress;
    std::string displayAddress;

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

        if (programVariables.count("port")){
            std::cerr << "Port number " << programVariables["port"].as<int>() << std::endl;
        }
    }
};

int main(int argc, char *argv[]) {
    ClientOptions client {argc, argv};

    std::cout << "Hello, World!" << std::endl;

    return 0;
}
