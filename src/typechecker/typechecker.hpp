

#include "../parser/parser.hpp"

namespace TypeChecker {

    Type processOperation(Node* node, Type parentType);
    Type processInvocation(Node* node);
    bool process(std::unordered_map<std::string, Node*>* tree);
}

