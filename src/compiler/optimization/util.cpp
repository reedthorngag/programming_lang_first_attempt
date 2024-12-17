#include <vector>
#include <string>
#include <unordered_map>

#include "util.hpp"
#include "../../types.hpp"
#include "../../parser/parser.hpp"

namespace Compiler {

    std::unordered_map<std::string, Node*>* getDependencies(Node* node) {
        return getDependencies(node, nullptr, new std::unordered_map<std::string, Node*>);
    }

    std::unordered_map<std::string, Node*>* getDependencies(Node* node, Node* parent, std::unordered_map<std::string, Node*>* externalDependencies) {
        
        switch (node->type) {
            case NodeType::SYMBOL:
            case NodeType::INVOCATION: {
                Symbol* symbol;
                if (!symbolDeclaredInScope(node->symbol->name,parent,&symbol)) {
                    externalDependencies->insert(std::make_pair(node->symbol->name,node));
                }
                break;
            }

            case NodeType::SCOPE:
            case NodeType::FUNCTION:
            case NodeType::IF:
            case NodeType::ELSE:
            case NodeType::WHILE:

            default:
                break;

        }

        if (node->firstChild) getDependencies(node->firstChild, parent, externalDependencies);

        if (node->nextSibling) getDependencies(node->nextSibling, parent, externalDependencies);

        return externalDependencies;
    }

}



