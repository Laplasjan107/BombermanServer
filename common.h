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
        boost::asio::read(socket, buffer(&number, sizeof(T)));
        boost::endian::endian_reverse_inplace(number);
    }
}

#endif //BOMBERMANSERVER_COMMON_H
