#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class ClientOptions {
    std::string clientName;
    uint16_t port;
    std::string serverAddress;
    std::string displayAddress;
};

int main(int argc, char *argv[]) {

    po::options_description description("MyTool Usage");
    description.add_options()
            ("help,h", "Display this help message")
            ("port,p", po::value<int>(), "Port number")
            ("version", "Display the version number");
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
    po::notify(vm);

    if(vm.count("port")){
        std::cout << "Port number " << vm["port"].as<int>() << std::endl;
    }

    std::cout << "Hello, World!" << std::endl;

    return 0;
}
