

#include "../parser/parser.hpp"

namespace TypeChecker {

    Parser::Type processOperation(Parser::Node* node);
    bool process(std::unordered_map<std::string, Parser::Node*>* tree);
}

