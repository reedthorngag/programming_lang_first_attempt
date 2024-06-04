#include <stdio.h>

#include "../parser/parser.hpp"

using namespace Parser;

namespace TypeChecker {

    bool process(std::unordered_map<std::string, Parser::Node*>* tree) {

        printf("hello from the type checker\n");

        for (auto& [key, node] : *tree) {
            switch (node->type) {
                case NodeType::FUNCTION:
                    break;
                case NodeType::SYMBOL:
                    break;
                default:
                    printf("ERROR: %s:%d:%d: expecting decleration or function, found '%s'!\n",node->token.file,node->token.line,node->token.column,NodeTypeMap[(int)node->type]);
                    return false;
            }
        }
        return true;
    }
}

