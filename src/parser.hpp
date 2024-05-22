
#include "lexer.hpp"

using namespace Lexer;

namespace Parser {

    struct Node {
        Node* parent;
        Node* child1;
        Node* child2;
        Token token;
    };

    int parseTokens(std::vector<Token>* tokens);
}


