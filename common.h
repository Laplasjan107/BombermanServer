//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_COMMON_H
#define BOMBERMANSERVER_COMMON_H

#include "types.h"
#include "event/Event.h"
#include <boost/endian/conversion.hpp>
#include <boost/asio.hpp>

namespace bomberman {
    template<typename T>
    void read_number_inplace(socket_t &socket, T &number) {
        using namespace boost::asio;

        read(socket, buffer(&number, sizeof(T)));
        boost::endian::endian_reverse_inplace(number);
    }

    string read_string(socket_t &socket) {
        using namespace boost::asio;

        string_length_t stringLength;
        read_number_inplace(socket, stringLength);

        auto readString = string(stringLength, '\0');
        read(socket, buffer(readString.data(), stringLength));
        return readString;
    }
}

#endif //BOMBERMANSERVER_COMMON_H
