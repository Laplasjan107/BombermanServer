//
// Created by Piotr Kamiński on 22/05/2022.
//

#ifndef BOMBERMANSERVER_POSITION_H
#define BOMBERMANSERVER_POSITION_H

#include <iostream>
#include "types.h"
#include "common.h"

namespace bomberman {
    struct Position {
        board_size_t positionX{};
        board_size_t positionY{};

        constexpr Position() = default;

        explicit Position(socket_t &socket) {
            read_number_inplace(socket, positionX);
            read_number_inplace(socket, positionY);
        }

        constexpr Position(board_size_t x, board_size_t y) : positionX(x), positionY(y) {
        }

        bool operator==(const Position &other) const {
            return (positionX == other.positionX) && (positionY == other.positionY);
        }

        Position operator+(Position const &other) const {
            Position added;
            added.positionX = positionX + other.positionX;
            added.positionY = positionY + other.positionY;
            return added;
        }

        Position operator*(explosion_radius_t multiplier) const {
            Position added;
            added.positionX = positionX * multiplier;
            added.positionY = positionY * multiplier;
            return added;
        }

        friend std::ostream &operator<<(std::ostream &os, const Position &position) {
            return os << '(' << position.positionX << ", " << position.positionY << ')';
        }
    };
}

namespace std {
    template<>
    struct hash<bomberman::Position> {
        size_t operator()(const bomberman::Position &position) const {
            return hash<bomberman::board_size_t>()(position.positionX) ^
                   hash<bomberman::board_size_t>()(position.positionY);
        }
    };

}

#endif //BOMBERMANSERVER_POSITION_H
