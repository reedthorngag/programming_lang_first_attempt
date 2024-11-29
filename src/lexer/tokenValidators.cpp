#include "lexer.hpp"
#include "util.hpp"

namespace Lexer {

    bool validateSymbol() {

        char* ptr = Lexer::ptr;
        int len;

        do {
            if (!symbolChar(ptr[len])) {
                if (!len) return false;

                if (!breakChar(ptr[len])) return false;

                tokens->push_back(Token{});

                Lexer::ptr += len;
            }
        }
    }
}