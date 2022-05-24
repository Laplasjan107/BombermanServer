//
// Created by Piotr Kamiński on 23/05/2022.
//

#ifndef BOMBERMANSERVER_UNEXPECTEDMESSAGEEXCEPTION_H
#define BOMBERMANSERVER_UNEXPECTEDMESSAGEEXCEPTION_H

#include <exception>

namespace bomberman {
    struct UnexpectedMessageException : public std::bad_cast {
        [[nodiscard]] const char *what() const noexcept override {
            return "Read invalid message from server";
        }
    };
}

#endif //BOMBERMANSERVER_UNEXPECTEDMESSAGEEXCEPTION_H
