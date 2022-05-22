//
// Created by Piotr Kamiński on 21/05/2022.
//

#ifndef BOMBERMANSERVER_COMMON_H
#define BOMBERMANSERVER_COMMON_H

#include "types.h"
#include <boost/endian/conversion.hpp>
#include <boost/asio.hpp>

namespace bomberman {
    template<typename T>
    void read_number_inplace(socket_t &socket, T &number) {
        void handler(
                // Result of operation.
                const boost::system::error_code& error,
                // Number of bytes copied into the buffers. If an error
                // occurred, this will be the number of bytes successfully
                // transferred prior to the error.
                std::size_t bytes_transferred
        );

        using namespace boost::asio;

        async_read(socket, buffer(&number, sizeof(T)), handler);
        boost::endian::endian_reverse_inplace(number);
    }

    string read_string(socket_t &socket) {
        using namespace boost::asio;

        string_length_t stringLength;
        read_number_inplace(socket, stringLength);

        auto readString = string(stringLength + 1, '\0');
        read(socket, buffer(readString.data(), stringLength));
        return readString;
    }
}

#endif //BOMBERMANSERVER_COMMON_H
